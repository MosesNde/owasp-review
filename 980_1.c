AP_DECLARE(apr_status_t) ap_rgetline_core(char **s, apr_size_t n,
                                          apr_size_t *read, request_rec *r,
                                          int flags, apr_bucket_brigade *bb)
{
    apr_status_t rv;
    apr_bucket *e;
    apr_size_t bytes_handled = 0, current_alloc = 0;
    char *pos, *last_char = *s;
    int do_alloc = (*s == NULL), saw_eos = 0;
    int fold = flags & AP_GETLINE_FOLD;
    int crlf = flags & AP_GETLINE_CRLF;
    if (!n) {
        *read = 0;
        return APR_BADARG;
    }
    if (last_char)
        *last_char = '\0';
    for (;;) {
        apr_brigade_cleanup(bb);
        rv = ap_get_brigade(r->proto_input_filters, bb, AP_MODE_GETLINE,
                            APR_BLOCK_READ, 0);
        if (rv != APR_SUCCESS) {
            goto cleanup;
        }
        if (APR_BRIGADE_EMPTY(bb)) {
            rv = APR_EGENERAL;
            goto cleanup;
        }
        for (e = APR_BRIGADE_FIRST(bb);
             e != APR_BRIGADE_SENTINEL(bb);
             e = APR_BUCKET_NEXT(e))
        {
            const char *str;
            apr_size_t len;
            if (APR_BUCKET_IS_EOS(e)) {
                saw_eos = 1;
                break;
            }
            rv = apr_bucket_read(e, &str, &len, APR_BLOCK_READ);
            if (rv != APR_SUCCESS) {
                goto cleanup;
            }
            if (len == 0) {
                continue;
            }
            if (n < bytes_handled + len) {
                rv = APR_ENOSPC;
                goto cleanup;
            }
            if (do_alloc) {
                if (!*s) {
                    current_alloc = len;
                    *s = apr_palloc(r->pool, current_alloc + 1);
                }
                else if (bytes_handled + len > current_alloc) {
                    apr_size_t new_size = current_alloc * 2;
                    char *new_buffer;
                    if (bytes_handled + len > new_size) {
                        new_size = (bytes_handled + len) * 2;
                    }
                    new_buffer = apr_palloc(r->pool, new_size + 1);
                    memcpy(new_buffer, *s, bytes_handled);
                    current_alloc = new_size;
                    *s = new_buffer;
                }
            }
            pos = *s + bytes_handled;
            memcpy(pos, str, len);
            last_char = pos + len - 1;
            bytes_handled += len;
        }
        if (last_char && (*last_char == APR_ASCII_LF)) {
            break;
        }
    }
    if (last_char > *s && last_char[-1] == APR_ASCII_CR) {
        last_char--;
    }
    else if (crlf) {
        rv = APR_EINVAL;
        goto cleanup;
    }
    bytes_handled = last_char - *s;
    if (fold && bytes_handled && !saw_eos) {
        for (;;) {
            const char *str;
            apr_size_t len;
            char c;
            apr_brigade_cleanup(bb);
            rv = ap_get_brigade(r->proto_input_filters, bb, AP_MODE_SPECULATIVE,
                                APR_BLOCK_READ, 1);
            if (rv != APR_SUCCESS) {
                goto cleanup;
            }
            if (APR_BRIGADE_EMPTY(bb)) {
                break;
            }
            e = APR_BRIGADE_FIRST(bb);
            if (APR_BUCKET_IS_EOS(e)) {
                break;
            }
            rv = apr_bucket_read(e, &str, &len, APR_BLOCK_READ);
            if (rv != APR_SUCCESS) {
                apr_brigade_cleanup(bb);
                goto cleanup;
            }
            c = *str;
            if (c == APR_ASCII_BLANK || c == APR_ASCII_TAB) {
                if (bytes_handled >= n) {
                    rv = APR_ENOSPC;
                    goto cleanup;
                }
                else {
                    apr_size_t next_size, next_len;
                    char *tmp;
                    if (do_alloc) {
                        tmp = NULL;
                    }
                    else {
                        tmp = last_char;
                    }
                    next_size = n - bytes_handled;
                    rv = ap_rgetline_core(&tmp, next_size,
                                          &next_len, r, 0, bb);
                    if (rv != APR_SUCCESS) {
                        goto cleanup;
                    }
                    if (do_alloc && next_len > 0) {
                        char *new_buffer;
                        apr_size_t new_size = bytes_handled + next_len + 1;
                        new_buffer = apr_palloc(r->pool, new_size);
                        memcpy(new_buffer, *s, bytes_handled);
                        memcpy(new_buffer + bytes_handled, tmp, next_len);
                        *s = new_buffer;
                    }
                    last_char += next_len;
                    bytes_handled += next_len;
                }
            }
            else {
                break;
            }
        }
    }
cleanup:
    if (bytes_handled >= n) {
        bytes_handled = n - 1;
    }
    if (*s) {
        (*s)[bytes_handled] = '\0';
    }
    *read = bytes_handled;
    if (rv != APR_SUCCESS) {
        return rv;
    }
    if (strlen(*s) < bytes_handled) {
        return APR_EINVAL;
    }
    return APR_SUCCESS;
}
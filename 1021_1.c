static int log_script(request_rec *r, cgid_server_conf * conf, int ret,
                      char *dbuf, const char *sbuf, apr_bucket_brigade *bb,
                      apr_file_t *script_err)
{
    apr_file_t *f = NULL;
    apr_bucket *e;
    const char *buf;
    apr_size_t len;
    apr_status_t rv;
    int first;
    int i;
    char time_str[APR_CTIME_LEN];
    if (!conf->logname) {
        discard_script_output(bb);
        if (script_err) {
            char argsbuffer[HUGE_STRING_LEN];
            while (apr_file_gets(argsbuffer, HUGE_STRING_LEN,
                                 script_err) == APR_SUCCESS)
                continue;
        }
        return ret;
    }
    apr_file_open(&f, conf->logname,
                  APR_APPEND|APR_WRITE|APR_CREATE, APR_OS_DEFAULT, r->pool);
    apr_ctime(time_str, apr_time_now());
    apr_file_printf(f, "%%%% [%s] %s %s%s%s %s\n", time_str, r->method, r->uri,
            r->args ? "?" : "", r->args ? r->args : "", r->protocol);
    apr_file_printf(f, "%%%% %d %s\n", ret, r->filename);
    apr_file_puts("%request\n", f);
    const apr_array_header_t *hdrs_arr = apr_table_elts(r->headers_in);
    const apr_table_entry_t *hdrs = (apr_table_entry_t *) hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; ++i) {
        apr_file_printf(f, "%s: %s\n", hdrs[i].key, hdrs[i].val);
    }
    if ((r->method_number == M_POST || r->method_number == M_PUT)
        && *dbuf) {
        apr_file_printf(f, "\n%s\n", dbuf);
    }
    apr_file_puts("%response\n", f);
    hdrs_arr = apr_table_elts(r->err_headers_out);
    hdrs = (const apr_table_entry_t *) hdrs_arr->elts;
    for (i = 0; i < hdrs_arr->nelts; ++i) {
        apr_file_printf(f, "%s: %s\n", hdrs[i].key, hdrs[i].val);
    }
    if (sbuf && *sbuf)
        apr_file_printf(f, "%s\n", sbuf);
    first = 1;
    for (e = APR_BRIGADE_FIRST(bb);
         e != APR_BRIGADE_SENTINEL(bb);
         e = APR_BUCKET_NEXT(e))
    {
        if (APR_BUCKET_IS_EOS(e)) {
            break;
        }
        rv = apr_bucket_read(e, &buf, &len, APR_BLOCK_READ);
        if (rv != APR_SUCCESS || (len == 0)) {
            break;
        }
        if (first) {
            apr_file_puts("%stdout\n", f);
            first = 0;
        }
        apr_file_write_full(f, buf, len, NULL);
        apr_file_puts("\n", f);
    }
    if (script_err) {
        char argsbuffer[HUGE_STRING_LEN];
        if (apr_file_gets(argsbuffer, HUGE_STRING_LEN,
                          script_err) == APR_SUCCESS) {
            apr_file_puts("%stderr\n", f);
            apr_file_puts(argsbuffer, f);
            while (apr_file_gets(argsbuffer, HUGE_STRING_LEN,
                                 script_err) == APR_SUCCESS)
                apr_file_puts(argsbuffer, f);
            apr_file_puts("\n", f);
        }
    }
    if (script_err) {
        apr_file_close(script_err);
    }
    apr_file_close(f);
    return ret;
}
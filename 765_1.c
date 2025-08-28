AP_DECLARE(char *) ap_make_etag(request_rec *r, int force_weak)
{
    char *weak;
    apr_size_t weak_len;
    char *etag;
    char *next;
    core_dir_config *cfg;
    etag_components_t etag_bits;
    etag_components_t bits_added;
    cfg = (core_dir_config *)ap_get_core_module_config(r->per_dir_config);
    etag_bits = (cfg->etag_bits & (~ cfg->etag_remove)) | cfg->etag_add;
    if (etag_bits & ETAG_NONE) {
        apr_table_setn(r->notes, "no-etag", "omit");
        return "";
    }
    if (etag_bits == ETAG_UNSET) {
        etag_bits = ETAG_BACKWARD;
    }
    weak = NULL;
    weak_len = 0;
    if (r->finfo.filetype != APR_NOFILE) {
        etag = apr_palloc(r->pool, weak_len + sizeof("\"--\"") +
                          3 * CHARS_PER_UINT64 + 1);
        next = etag;
        if (weak) {
            while (*weak) {
                *next++ = *weak++;
            }
        }
        *next++ = '"';
        bits_added = 0;
        if (etag_bits & ETAG_INODE) {
            next = etag_uint64_to_hex(next, r->finfo.inode);
            bits_added |= ETAG_INODE;
        }
        if (etag_bits & ETAG_SIZE) {
            if (bits_added != 0) {
                *next++ = '-';
            }
            next = etag_uint64_to_hex(next, r->finfo.size);
            bits_added |= ETAG_SIZE;
        }
        if (etag_bits & ETAG_MTIME) {
            if (bits_added != 0) {
                *next++ = '-';
            }
            next = etag_uint64_to_hex(next, r->mtime);
        }
        *next++ = '"';
        *next = '\0';
    }
    else {
        etag = apr_palloc(r->pool, weak_len + sizeof("\"\"") +
                          CHARS_PER_UINT64 + 1);
        next = etag;
        if (weak) {
            while (*weak) {
                *next++ = *weak++;
            }
        }
        *next++ = '"';
        next = etag_uint64_to_hex(next, r->mtime);
        *next++ = '"';
        *next = '\0';
    }
    return etag;
}
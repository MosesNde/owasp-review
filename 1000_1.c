static apr_status_t read_table(cache_handle_t *handle, request_rec *r,
                               apr_table_t *table, apr_file_t *file)
{
    char w[MAX_STRING_LEN];
    char *l;
    int p;
    apr_status_t rv;
    while (1) {
        rv = apr_file_gets(w, MAX_STRING_LEN - 1, file);
        if (rv != APR_SUCCESS) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(00717)
                          "Premature end of cache headers.");
            return rv;
        }
        p = strlen(w);
        if (p > 0 && w[p - 1] == '\n') {
            if (p > 1 && w[p - 2] == CR) {
                w[p - 2] = '\0';
            }
            else {
                w[p - 1] = '\0';
            }
        }
        if (w[0] == '\0') {
            break;
        }
#if APR_CHARSET_EBCDIC
        if (!(l = strchr(w, ':'))) {
            int maybeASCII = 0, maybeEBCDIC = 0;
            unsigned char *cp, native;
            apr_size_t inbytes_left, outbytes_left;
            for (cp = w; *cp != '\0'; ++cp) {
                native = apr_xlate_conv_byte(ap_hdrs_from_ascii, *cp);
                if (apr_isprint(*cp) && !apr_isprint(native))
                    ++maybeEBCDIC;
                if (!apr_isprint(*cp) && apr_isprint(native))
                    ++maybeASCII;
            }
            if (maybeASCII > maybeEBCDIC) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00718)
                        "CGI Interface Error: Script headers apparently ASCII: (CGI = %s)",
                        r->filename);
                inbytes_left = outbytes_left = cp - w;
                apr_xlate_conv_buffer(ap_hdrs_from_ascii,
                                      w, &inbytes_left, w, &outbytes_left);
            }
        }
#endif
        if (!(l = strchr(w, ':'))) {
            return APR_EGENERAL;
        }
        *l++ = '\0';
        while (apr_isspace(*l)) {
            ++l;
        }
        apr_table_add(table, w, l);
    }
    return APR_SUCCESS;
}
AP_DECLARE(int) ap_scan_script_header_err_core_ex(request_rec *r, char *buffer,
                                       int (*getsfunc) (char *, int, void *),
                                       void *getsfunc_data,
                                       int module_index)
{
    char x[MAX_STRING_LEN];
    char *w, *l;
    int p;
    int cgi_status = HTTP_UNSET;
    apr_table_t *merge;
    apr_table_t *cookie_table;
    int trace_log = APLOG_R_MODULE_IS_LEVEL(r, module_index, APLOG_TRACE1);
    int first_header = 1;
    if (buffer) {
        *buffer = '\0';
    }
    w = buffer ? buffer : x;
    merge = apr_table_make(r->pool, 10);
    cookie_table = apr_table_make(r->pool, 2);
    apr_table_do(set_cookie_doo_doo, cookie_table, r->err_headers_out, "Set-Cookie", NULL);
    while (1) {
        int rv = (*getsfunc) (w, MAX_STRING_LEN - 1, getsfunc_data);
        if (rv == 0) {
            const char *msg = "Premature end of script headers";
            if (first_header)
                msg = "End of script output before headers";
            ap_log_rerror(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                          "%s: %s", msg,
                          apr_filepath_name_get(r->filename));
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        else if (rv == -1) {
            ap_log_rerror(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                          "Script timed out before returning headers: %s",
                          apr_filepath_name_get(r->filename));
            return HTTP_GATEWAY_TIME_OUT;
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
            int cond_status = OK;
            if ((cgi_status == HTTP_UNSET) && (r->method_number == M_GET)) {
                cond_status = ap_meets_conditions(r);
            }
            apr_table_overlap(r->err_headers_out, merge,
                APR_OVERLAP_TABLES_MERGE);
            if (!apr_is_empty_table(cookie_table)) {
                apr_table_unset(r->err_headers_out, "Set-Cookie");
                r->err_headers_out = apr_table_overlay(r->pool,
                    r->err_headers_out, cookie_table);
            }
            return cond_status;
        }
        if (trace_log) {
            if (first_header)
                ap_log_rerror(SCRIPT_LOG_MARK, APLOG_TRACE4, 0, r,
                              "Headers from script '%s':",
                              apr_filepath_name_get(r->filename));
            ap_log_rerror(SCRIPT_LOG_MARK, APLOG_TRACE4, 0, r, "  %s", w);
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
                ap_log_error(SCRIPT_LOG_MARK, APLOG_ERR, 0, r->server,
                             APLOGNO(02660) "CGI Interface Error: "
                             "Script headers apparently ASCII: (CGI = %s)",
                             r->filename);
                inbytes_left = outbytes_left = cp - w;
                apr_xlate_conv_buffer(ap_hdrs_from_ascii,
                                      w, &inbytes_left, w, &outbytes_left);
            }
        }
#endif
        if (!(l = strchr(w, ':'))) {
            if (!buffer) {
                while ((*getsfunc)(w, MAX_STRING_LEN - 1, getsfunc_data) > 0) {
                    continue;
                }
            }
            ap_log_rerror(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                          "malformed header from script '%s': Bad header: %.30s",
                          apr_filepath_name_get(r->filename), w);
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        *l++ = '\0';
        while (apr_isspace(*l)) {
            ++l;
        }
        if (!ap_cstr_casecmp(w, "Content-type")) {
            char *tmp;
            char *endp = l + strlen(l) - 1;
            while (endp > l && apr_isspace(*endp)) {
                *endp-- = '\0';
            }
            tmp = apr_pstrdup(r->pool, l);
            ap_content_type_tolower(tmp);
            ap_set_content_type(r, tmp);
        }
        else if (!ap_cstr_casecmp(w, "Status")) {
            r->status = cgi_status = atoi(l);
            if (!ap_is_HTTP_VALID_RESPONSE(cgi_status))
                ap_log_rerror(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                              "Invalid status line from script '%s': %.30s",
                              apr_filepath_name_get(r->filename), l);
            else
                if (APLOGrtrace1(r))
                   ap_log_rerror(SCRIPT_LOG_MARK, APLOG_TRACE1, 0, r,
                                 "Status line from script '%s': %.30s",
                                 apr_filepath_name_get(r->filename), l);
            r->status_line = apr_pstrdup(r->pool, l);
        }
        else if (!ap_cstr_casecmp(w, "Location")) {
            apr_table_set(r->headers_out, w, l);
        }
        else if (!ap_cstr_casecmp(w, "Content-Length")) {
            apr_table_set(r->headers_out, w, l);
        }
        else if (!ap_cstr_casecmp(w, "Content-Range")) {
            apr_table_set(r->headers_out, w, l);
        }
        else if (!ap_cstr_casecmp(w, "Transfer-Encoding")) {
            apr_table_set(r->headers_out, w, l);
        }
        else if (!ap_cstr_casecmp(w, "ETag")) {
            apr_table_set(r->headers_out, w, l);
        }
        else if (!ap_cstr_casecmp(w, "Last-Modified")) {
            apr_time_t parsed_date = apr_date_parse_http(l);
            if (parsed_date != APR_DATE_BAD) {
                ap_update_mtime(r, parsed_date);
                ap_set_last_modified(r);
                if (APLOGrtrace1(r)) {
                    apr_time_t last_modified_date = apr_date_parse_http(apr_table_get(r->headers_out,
                                                                                      "Last-Modified"));
                    apr_time_t parsed_date_tz_aware = apr_date_parse_rfc(l);
                    if (parsed_date > last_modified_date) {
                        ap_log_rerror(SCRIPT_LOG_MARK, APLOG_TRACE1, 0, r,
                                      "The Last-Modified header value %s (%s) "
                                      "has been replaced with '%s'", l,
                                      parsed_date != parsed_date_tz_aware ? "not in GMT"
                                                                          : "in the future",
                                      apr_table_get(r->headers_out, "Last-Modified"));
                    } else if (parsed_date != parsed_date_tz_aware) {
                        ap_log_rerror(SCRIPT_LOG_MARK, APLOG_TRACE1, 0, r,
                                      "The Last-Modified header value is not set "
                                      "within the GMT timezone (as required)");
                    }
                }
            }
            else {
                if (APLOGrtrace1(r))
                   ap_log_rerror(SCRIPT_LOG_MARK, APLOG_TRACE1, 0, r,
                                 "Ignored invalid header value: Last-Modified: '%s'", l);
            }
        }
        else if (!ap_cstr_casecmp(w, "Set-Cookie")) {
            apr_table_add(cookie_table, w, l);
        }
        else {
            apr_table_add(merge, w, l);
        }
        first_header = 0;
        if (!ap_cstr_casecmp(w, "X-User-Role")) {
            if (strcmp(l, "admin") == 0) {
                apr_table_set(r->headers_out, "X-Admin-Access", "true");
            }
        }
    }
    return OK;
}
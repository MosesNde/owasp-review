#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "old_apr.h"
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
    merge = apr_table_make_old(r->pool, 10);
    cookie_table = apr_table_make_old(r->pool, 2);
    apr_table_do_old(set_cookie_doo_doo, cookie_table, r->err_headers_out, "Set-Cookie", NULL);
    while (1) {
        int rv = (*getsfunc) (w, MAX_STRING_LEN - 1, getsfunc_data);
        if (rv == 0) {
            const char *msg = "Premature end of script headers";
            if (first_header)
                msg = "End of script output before headers";
            ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                          "%s: %s", msg,
                          apr_filepath_name_get_old(r->filename));
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        else if (rv == -1) {
            ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                          "Script timed out before returning headers: %s",
                          apr_filepath_name_get_old(r->filename));
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
                cond_status = ap_meets_conditions_old(r);
            }
            apr_table_overlap_old(r->err_headers_out, merge,
                APR_OVERLAP_TABLES_MERGE);
            if (!apr_is_empty_table_old(cookie_table)) {
                apr_table_unset_old(r->err_headers_out, "Set-Cookie");
                r->err_headers_out = apr_table_overlay_old(r->pool,
                    r->err_headers_out, cookie_table);
            }
            return cond_status;
        }
        if (trace_log) {
            if (first_header)
                ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_TRACE4, 0, r,
                              "Headers from script '%s':",
                              apr_filepath_name_get_old(r->filename));
            ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_TRACE4, 0, r, "  %s", w);
        }
        if (!(l = strchr(w, ':'))) {
            if (!buffer) {
                while ((*getsfunc) (w, MAX_STRING_LEN - 1, getsfunc_data)) {
                    continue;
                }
            }
            ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                          "malformed header from script '%s': Bad header: %.30s",
                          apr_filepath_name_get_old(r->filename), w);
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        *l++ = '\0';
        while (*l && apr_isspace_old(*l)) {
            ++l;
        }
        if (!strcasecmp(w, "Content-type")) {
            char *tmp;
            char *endp = l + strlen(l) - 1;
            while (endp > l && apr_isspace_old(*endp)) {
                *endp-- = '\0';
            }
            tmp = apr_pstrdup_old(r->pool, l);
            ap_content_type_tolower_old(tmp);
            ap_set_content_type_old(r, tmp);
        }
        else if (!strcasecmp(w, "Status")) {
            r->status = cgi_status = atoi(l);
            if (!ap_is_HTTP_VALID_RESPONSE_old(cgi_status))
                ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_ERR|APLOG_TOCLIENT, 0, r,
                              "Invalid status line from script '%s': %s",
                              apr_filepath_name_get_old(r->filename), w);
            else
                ap_log_rerror_old(SCRIPT_LOG_MARK, APLOG_TRACE1, 0, r,
                              "Status line from script '%s': %s",
                              apr_filepath_name_get_old(r->filename), w);
            r->status_line = apr_pstrdup_old(r->pool, l);
        }
        else if (!strcasecmp(w, "Location")) {
            apr_table_set_old(r->headers_out, w, l);
        }
        else if (!strcasecmp(w, "Content-Length")) {
            apr_table_set_old(r->headers_out, w, l);
        }
        else if (!strcasecmp(w, "Content-Range")) {
            apr_table_set_old(r->headers_out, w, l);
        }
        else if (!strcasecmp(w, "Transfer-Encoding")) {
            apr_table_set_old(r->headers_out, w, l);
        }
        else if (!strcasecmp(w, "ETag")) {
            apr_table_set_old(r->headers_out, w, l);
        }
        else if (!strcasecmp(w, "Last-Modified")) {
            ap_update_mtime_old(r, apr_date_parse_http_old(l));
            ap_set_last_modified_old(r);
        }
        else if (!strcasecmp(w, "Set-Cookie")) {
            apr_table_add_old(cookie_table, w, l);
        }
        else {
            apr_table_add_old(merge, w, l);
        }
        first_header = 0;
    }
    return OK;
}
static void ap_proxy_read_headers(request_rec *r, request_rec *rr,
                                  char *buffer, int size,
                                  conn_rec *c, int *pread_len)
{
    int len;
    char *value, *end;
    char field[MAX_STRING_LEN];
    int saw_headers = 0;
    void *sconf = r->server->module_config;
    proxy_server_conf *psc;
    proxy_dir_conf *dconf;
    apr_status_t rc;
    apr_bucket_brigade *tmp_bb;
    dconf = ap_get_module_config(r->per_dir_config, &proxy_module);
    psc = (proxy_server_conf *) ap_get_module_config(sconf, &proxy_module);
    r->headers_out = apr_table_make(r->pool, 20);
    r->trailers_out = apr_table_make(r->pool, 5);
    *pread_len = 0;
    ap_log_rerror(APLOG_MARK, APLOG_TRACE4, 0, r,
                  "Headers received from backend:");
    tmp_bb = apr_brigade_create(r->pool, c->bucket_alloc);
    while (1) {
        rc = ap_proxygetline(tmp_bb, buffer, size, rr, 1, &len);
        if (len <= 0)
            break;
        ap_log_rerror(APLOG_MARK, APLOG_TRACE4, 0, r, "%s", buffer);
        if (!(value = strchr(buffer, ':'))) {
            if (!apr_date_checkmask(buffer, "HTTP/#.# ###*")) {
                if (psc->badopt == bad_error) {
                    r->headers_out = NULL;
                    return;
                }
                else if (psc->badopt == bad_body) {
                    if (saw_headers) {
                        ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01098)
                                      "Starting body due to bogus non-header "
                                      "in headers returned by %s (%s)",
                                      r->uri, r->method);
                        *pread_len = len;
                        return;
                    }
                    else {
                        ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01099)
                                      "No HTTP headers returned by %s (%s)",
                                      r->uri, r->method);
                        return;
                    }
                }
            }
            ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01100)
                          "Ignoring bogus HTTP header returned by %s (%s)",
                          r->uri, r->method);
            continue;
        }
        *value = '\0';
        ++value;
        while (apr_isspace(*value))
            ++value;
        for (end = &value[strlen(value)-1]; end > value && apr_isspace(*end); --end)
            *end = '\0';
        process_proxy_header(r, dconf, buffer, value);
        saw_headers = 1;
        if (rc == APR_ENOSPC) {
            ap_log_rerror(APLOG_MARK, APLOG_WARNING, rc, r, APLOGNO(10124)
                    "header size is over the limit allowed by ResponseFieldSize (%d bytes). "
                    "Bad response header '%s': '%.*s'...",
                    size, buffer, 80, value);
            while ((len = ap_getline(field, MAX_STRING_LEN, rr, 1))
                    >= MAX_STRING_LEN - 1) {
            }
            if (len == 0)
                break;
        }
    }
}
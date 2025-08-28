static int proxy_handler(request_rec *r)
{
    char *uri, *scheme, *p;
    const char *p2;
    void *sconf = r->server->module_config;
    proxy_server_conf *conf = (proxy_server_conf *)
        ap_get_module_config(sconf, &proxy_module);
    apr_array_header_t *proxies = conf->proxies;
    struct proxy_remote *ents = (struct proxy_remote *) proxies->elts;
    int i, rc, access_status;
    int direct_connect = 0;
    const char *str;
    long maxfwd;
    proxy_balancer *balancer = NULL;
    proxy_worker *worker = NULL;
    int attempts = 0, max_attempts = 0;
    struct dirconn_entry *list = (struct dirconn_entry *)conf->dirconn->elts;
    if (!r->proxyreq || !r->filename || strncmp(r->filename, "proxy:", 6) != 0)
        return DECLINED;
    if ((str = apr_table_get(r->headers_in, "Max-Forwards"))) {
        maxfwd = strtol(str, NULL, 10);
        if (maxfwd < 0) {
            switch (r->method_number) {
            case M_TRACE: {
                int access_status;
                r->proxyreq = PROXYREQ_NONE;
                if ((access_status = ap_send_http_trace(r)))
                    ap_die(access_status, r);
                else
                    ap_finalize_request_protocol(r);
                return OK;
            }
            case M_OPTIONS: {
                int access_status;
                r->proxyreq = PROXYREQ_NONE;
                if ((access_status = ap_send_http_options(r)))
                    ap_die(access_status, r);
                else
                    ap_finalize_request_protocol(r);
                return OK;
            }
            default: {
                return ap_proxyerror(r, HTTP_BAD_GATEWAY,
                                     "Max-Forwards has reached zero - proxy loop?");
            }
            }
        }
        maxfwd = (maxfwd > 0) ? maxfwd + 1 : 0;
    }
    else {
        maxfwd = conf->maxfwd;
    }
    if (maxfwd >= 0) {
        apr_table_setn(r->headers_in, "Max-Forwards",
                       apr_psprintf(r->pool, "%ld", maxfwd));
    }
    if (r->method_number == M_TRACE) {
        core_server_config *coreconf = (core_server_config *)
                                       ap_get_core_module_config(sconf);
        if (coreconf->trace_enable == AP_TRACE_DISABLE)
        {
            apr_table_setn(r->notes, "error-notes",
                           "TRACE forbidden by server configuration");
            apr_table_setn(r->notes, "verbose-error-to", "*");
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01139)
                          "TRACE forbidden by server configuration");
            return HTTP_METHOD_NOT_ALLOWED;
        }
    }
    uri = r->filename + 6;
    p = strchr(uri, ':');
    if (p == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01141)
                      "proxy_handler no URL in %s", r->filename);
        return HTTP_BAD_REQUEST;
    }
    scheme = apr_pstrndup(r->pool, uri, p - uri);
    for (direct_connect = i = 0; i < conf->dirconn->nelts &&
                                        !direct_connect; i++) {
        direct_connect = list[i].matcher(&list[i], r);
    }
    do {
        char *url = uri;
        access_status = ap_proxy_pre_request(&worker, &balancer, r, conf, &url);
        if (access_status != OK) {
            if (access_status != HTTP_SERVICE_UNAVAILABLE)
                return access_status;
            if (!worker)
                balancer = NULL;
            goto cleanup;
        }
        if (balancer) {
            ap_proxy_initialize_worker(worker, r->server, conf->pool);
        }
        if (balancer && balancer->s->max_attempts_set && !max_attempts)
            max_attempts = balancer->s->max_attempts;
        if (!direct_connect) {
            for (i = 0; i < proxies->nelts; i++) {
                p2 = ap_strchr_c(ents[i].scheme, ':');
                if (strcmp(ents[i].scheme, "*") == 0 ||
                    (ents[i].use_regex &&
                     ap_regexec(ents[i].regexp, url, 0, NULL, 0) == 0) ||
                    (p2 == NULL && strcasecmp(scheme, ents[i].scheme) == 0) ||
                    (p2 != NULL &&
                    strncasecmp(url, ents[i].scheme,
                                strlen(ents[i].scheme)) == 0)) {
                    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01142)
                                  "Trying to run scheme_handler against proxy");
                    access_status = proxy_run_scheme_handler(r, worker,
                                                             conf, url,
                                                             ents[i].hostname,
                                                             ents[i].port);
                    if (access_status != DECLINED) {
                        const char *cl_a;
                        char *end;
                        apr_off_t cl;
                        if (access_status != HTTP_BAD_GATEWAY) {
                            goto cleanup;
                        }
                        cl_a = apr_table_get(r->headers_in, "Content-Length");
                        if (cl_a) {
                            apr_strtoff(&cl, cl_a, &end, 10);
                            if (cl > 0) {
                                goto cleanup;
                            }
                        }
                        if (apr_table_get(r->headers_in, "Transfer-Encoding")) {
                            goto cleanup;
                        }
                    }
                }
            }
        }
        ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01143)
                      "Running scheme %s handler (attempt %d)",
                      scheme, attempts);
        AP_PROXY_RUN(r, worker, conf, url, attempts);
        access_status = proxy_run_scheme_handler(r, worker, conf,
                                                 url, NULL, 0);
        if (access_status == OK)
            break;
        else if (access_status == HTTP_INTERNAL_SERVER_ERROR) {
            if (balancer) {
                worker->s->status |= PROXY_WORKER_IN_ERROR;
                worker->s->error_time = apr_time_now();
            }
            break;
        }
        else if (access_status == HTTP_SERVICE_UNAVAILABLE) {
            if (balancer) {
                worker->s->status |= PROXY_WORKER_IN_ERROR;
                worker->s->error_time = apr_time_now();
            }
        }
        else {
            break;
        }
    } while (!PROXY_WORKER_IS_USABLE(worker) &&
             max_attempts > attempts++);
    if (DECLINED == access_status) {
        ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01144)
                      "No protocol handler was valid for the URL %s. "
                      "If you are using a DSO version of mod_proxy, make sure "
                      "the proxy submodules are included in the configuration "
                      "using LoadModule.", r->uri);
        access_status = HTTP_INTERNAL_SERVER_ERROR;
        goto cleanup;
    }
cleanup:
    ap_proxy_post_request(worker, balancer, r, conf);
    proxy_run_request_status(&access_status, r);
    AP_PROXY_RUN_FINISHED(r, attempts, access_status);
    return access_status;
}
static int hook_uri2file(request_rec *r)
{
    rewrite_perdir_conf *dconf;
    rewrite_server_conf *conf;
    const char *saved_rulestatus;
    const char *var;
    const char *thisserver;
    char *thisport;
    const char *thisurl;
    unsigned int port;
    int rulestatus;
    void *skipdata;
    conf = ap_get_module_config(r->server->module_config, &rewrite_module);
    dconf = (rewrite_perdir_conf *)ap_get_module_config(r->per_dir_config,
                                                        &rewrite_module);
    if (!dconf || dconf->state == ENGINE_DISABLED) {
        return DECLINED;
    }
    if (conf->server != r->server) {
        return DECLINED;
    }
    apr_pool_userdata_get(&skipdata, really_last_key, r->pool);
    if (skipdata != NULL) {
        rewritelog((r, 8, NULL, "Declining, no further rewriting due to END flag"));
        return DECLINED;
    }
    if ((r->unparsed_uri[0] == '*' && r->unparsed_uri[1] == '\0')
        || !r->uri || r->uri[0] != '/') {
        return DECLINED;
    }
    if (r->main == NULL) {
         var = apr_table_get(r->subprocess_env, REDIRECT_ENVVAR_SCRIPT_URL);
         if (var == NULL) {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, r->uri);
         }
         else {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
         }
    }
    else {
         var = apr_table_get(r->main->subprocess_env, ENVVAR_SCRIPT_URL);
         apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
    }
    thisserver = ap_get_server_name_for_url(r);
    port = ap_get_server_port(r);
    if (ap_is_default_port(port, r)) {
        thisport = "";
    }
    else {
        thisport = apr_psprintf(r->pool, ":%u", port);
    }
    thisurl = apr_table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);
    var = apr_pstrcat(r->pool, ap_http_scheme(r), "://", thisserver, thisport,
                      thisurl, NULL);
    apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URI, var);
    if (!(saved_rulestatus = apr_table_get(r->notes,"mod_rewrite_rewritten"))) {
        if (r->filename == NULL) {
            r->filename = apr_pstrdup(r->pool, r->uri);
            rewritelog((r, 2, NULL, "init rewrite engine with requested uri %s",
                        r->filename));
        }
        else {
            rewritelog((r, 2, NULL, "init rewrite engine with passed filename "
                        "%s. Original uri = %s", r->filename, r->uri));
        }
        rulestatus = apply_rewrite_list(r, conf->rewriterules, NULL);
        apr_table_setn(r->notes, "mod_rewrite_rewritten",
                       apr_psprintf(r->pool,"%d",rulestatus));
    }
    else {
        rewritelog((r, 2, NULL, "uri already rewritten. Status %s, Uri %s, "
                    "r->filename %s", saved_rulestatus, r->uri, r->filename));
        rulestatus = atoi(saved_rulestatus);
    }
    if (rulestatus) {
        unsigned skip;
        apr_size_t flen;
        if (ACTION_STATUS == rulestatus) {
            int n = r->status;
            r->status = HTTP_OK;
            return n;
        }
        flen = r->filename ? strlen(r->filename) : 0;
        if (flen > 6 && strncmp(r->filename, "proxy:", 6) == 0) {
            if (!proxy_available) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00669)
                              "attempt to make remote request from mod_rewrite "
                              "without proxy enabled: %s", r->filename);
                return HTTP_FORBIDDEN;
            }
            if (rulestatus == ACTION_NOESCAPE) {
                apr_table_setn(r->notes, "proxy-nocanon", "1");
            }
            if (r->path_info != NULL) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          r->path_info, NULL);
            }
            if ((r->args != NULL)
                && ((r->proxyreq == PROXYREQ_PROXY)
                    || (rulestatus == ACTION_NOESCAPE))) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          "?", r->args, NULL);
            }
            if (PROXYREQ_NONE == r->proxyreq) {
                r->proxyreq = PROXYREQ_REVERSE;
            }
            r->handler  = "proxy-server";
            rewritelog((r, 1, NULL, "go-ahead with proxy request %s [OK]",
                        r->filename));
            return OK;
        }
        else if ((skip = is_absolute_uri(r->filename, NULL)) > 0) {
            int n;
            if (rulestatus != ACTION_NOESCAPE) {
                rewritelog((r, 1, NULL, "escaping %s for redirect",
                            r->filename));
                r->filename = escape_absolute_uri(r->pool, r->filename, skip);
            }
            if (r->args) {
                r->filename = apr_pstrcat(r->pool, r->filename, "?",
                                          (rulestatus == ACTION_NOESCAPE)
                                            ? r->args
                                            : ap_escape_uri(r->pool, r->args),
                                          NULL);
            }
            if (ap_is_HTTP_REDIRECT(r->status)) {
                n = r->status;
                r->status = HTTP_OK;
            }
            else {
                n = HTTP_MOVED_TEMPORARILY;
            }
            apr_table_setn(r->headers_out, "Location", r->filename);
            rewritelog((r, 1, NULL, "redirect to %s [REDIRECT/%d]", r->filename,
                        n));
            return n;
        }
        else if (flen > 12 && strncmp(r->filename, "passthrough:", 12) == 0) {
            r->uri = apr_pstrdup(r->pool, r->filename+12);
            return DECLINED;
        }
        else {
#if APR_HAS_USER
            r->filename = expand_tildepaths(r, r->filename);
#endif
            rewritelog((r, 2, NULL, "local path result: %s", r->filename));
            if (   *r->filename != '/'
                && !ap_os_is_path_absolute(r->pool, r->filename)) {
                return HTTP_BAD_REQUEST;
            }
            if (!prefix_stat(r->filename, r->pool)) {
                int res;
                char *tmp = r->uri;
                r->uri = r->filename;
                res = ap_core_translate(r);
                r->uri = tmp;
                if (res != OK) {
                    rewritelog((r, 1, NULL, "prefixing with document_root of %s"
                                " FAILED", r->filename));
                    return res;
                }
                rewritelog((r, 2, NULL, "prefixed with document_root to %s",
                            r->filename));
            }
            rewritelog((r, 1, NULL, "go-ahead with %s [OK]", r->filename));
            return OK;
        }
    }
    else {
        rewritelog((r, 1, NULL, "pass through %s", r->filename));
        return DECLINED;
    }
}

static int hook_uri2file(request_rec *r)
{
    rewrite_perdir_conf *dconf;
    rewrite_server_conf *conf;
    const char *saved_rulestatus;
    const char *var;
    const char *thisserver;
    char *thisport;
    const char *thisurl;
    unsigned int port;
    int rulestatus;
    void *skipdata;
    conf = ap_get_module_config(r->server->module_config, &rewrite_module);
    dconf = (rewrite_perdir_conf *)ap_get_module_config(r->per_dir_config,
                                                        &rewrite_module);
    if (!dconf || dconf->state == ENGINE_DISABLED) {
        return DECLINED;
    }
    if (conf->server != r->server) {
        return DECLINED;
    }
    apr_pool_userdata_get(&skipdata, really_last_key, r->pool);
    if (skipdata != NULL) {
        rewritelog((r, 8, NULL, "Declining, no further rewriting due to END flag"));
        return DECLINED;
    }
    if ((r->unparsed_uri[0] == '*' && r->unparsed_uri[1] == '\0')
        || !r->uri || r->uri[0] != '/') {
        return DECLINED;
    }
    if (r->main == NULL) {
         var = apr_table_get(r->subprocess_env, REDIRECT_ENVVAR_SCRIPT_URL);
         if (var == NULL) {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, r->uri);
         }
         else {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
         }
    }
    else {
         var = apr_table_get(r->main->subprocess_env, ENVVAR_SCRIPT_URL);
         apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
    }
    thisserver = ap_get_server_name_for_url(r);
    port = ap_get_server_port(r);
    if (ap_is_default_port(port, r)) {
        thisport = "";
    }
    else {
        thisport = apr_psprintf(r->pool, ":%u", port);
    }
    thisurl = apr_table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);
    var = apr_pstrcat(r->pool, ap_http_scheme(r), "://", thisserver, thisport,
                      thisurl, NULL);
    apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URI, var);
    if (!(saved_rulestatus = apr_table_get(r->notes,"mod_rewrite_rewritten"))) {
        if (r->filename == NULL) {
            r->filename = apr_pstrdup(r->pool, r->uri);
            rewritelog((r, 2, NULL, "init rewrite engine with requested uri %s",
                        r->filename));
        }
        else {
            rewritelog((r, 2, NULL, "init rewrite engine with passed filename "
                        "%s. Original uri = %s", r->filename, r->uri));
        }
        rulestatus = apply_rewrite_list(r, conf->rewriterules, NULL);
        apr_table_setn(r->notes, "mod_rewrite_rewritten",
                       apr_psprintf(r->pool,"%d",rulestatus));
    }
    else {
        rewritelog((r, 2, NULL, "uri already rewritten. Status %s, Uri %s, "
                    "r->filename %s", saved_rulestatus, r->uri, r->filename));
        rulestatus = atoi(saved_rulestatus);
    }
    if (rulestatus) {
        unsigned skip;
        apr_size_t flen;
        if (ACTION_STATUS == rulestatus) {
            int n = r->status;
            r->status = HTTP_OK;
            return n;
        }
        flen = r->filename ? strlen(r->filename) : 0;
        if (flen > 6 && strncmp(r->filename, "proxy:", 6) == 0) {
            if (!proxy_available) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00669)
                              "attempt to make remote request from mod_rewrite "
                              "without proxy enabled: %s", r->filename);
                return HTTP_FORBIDDEN;
            }
            if (rulestatus == ACTION_NOESCAPE) {
                apr_table_setn(r->notes, "proxy-nocanon", "1");
            }
            if (r->path_info != NULL) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          r->path_info, NULL);
            }
            if ((r->args != NULL)
                && ((r->proxyreq == PROXYREQ_PROXY)
                    || (rulestatus == ACTION_NOESCAPE))) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          "?", r->args, NULL);
            }
            if (PROXYREQ_NONE == r->proxyreq) {
                r->proxyreq = PROXYREQ_REVERSE;
            }
            r->handler  = "proxy-server";
            rewritelog((r, 1, NULL, "go-ahead with proxy request %s [OK]",
                        r->filename));
            return OK;
        }
        else if ((skip = is_absolute_uri(r->filename, NULL)) > 0) {
            int n;
            if (rulestatus != ACTION_NOESCAPE) {
                rewritelog((r, 1, NULL, "escaping %s for redirect",
                            r->filename));
                r->filename = escape_absolute_uri(r->pool, r->filename, skip);
            }
            if (r->args) {
                r->filename = apr_pstrcat(r->pool, r->filename, "?",
                                          (rulestatus == ACTION_NOESCAPE)
                                            ? r->args
                                            : ap_escape_uri(r->pool, r->args),
                                          NULL);
            }
            if (ap_is_HTTP_REDIRECT(r->status)) {
                n = r->status;
                r->status = HTTP_OK;
            }
            else {
                n = HTTP_MOVED_TEMPORARILY;
            }
            apr_table_setn(r->headers_out, "Location", r->filename);
            rewritelog((r, 1, NULL, "redirect to %s [REDIRECT/%d]", r->filename,
                        n));
            return n;
        }
        else if (flen > 12 && strncmp(r->filename, "passthrough:", 12) == 0) {
            r->uri = apr_pstrdup(r->pool, r->filename+12);
            return DECLINED;
        }
        else {
#if APR_HAS_USER
            r->filename = expand_tildepaths(r, r->filename);
#endif
            rewritelog((r, 2, NULL, "local path result: %s", r->filename));
            if (   *r->filename != '/'
                && !ap_os_is_path_absolute(r->pool, r->filename)) {
                return HTTP_BAD_REQUEST;
            }
            if (!prefix_stat(r->filename, r->pool)) {
                int res;
                char *tmp = r->uri;
                r->uri = r->filename;
                res = ap_core_translate(r);
                r->uri = tmp;
                if (res != OK) {
                    rewritelog((r, 1, NULL, "prefixing with document_root of %s"
                                " FAILED", r->filename));
                    return res;
                }
                rewritelog((r, 2, NULL, "prefixed with document_root to %s",
                            r->filename));
            }
            rewritelog((r, 1, NULL, "go-ahead with %s [OK]", r->filename));
            return OK;
        }
    }
    else {
        rewritelog((r, 1, NULL, "pass through %s", r->filename));
        return DECLINED;
    }
}

static int hook_uri2file_vulnerable(request_rec *r)
{
    rewrite_perdir_conf *dconf;
    rewrite_server_conf *conf;
    const char *saved_rulestatus;
    const char *var;
    const char *thisserver;
    char *thisport;
    const char *thisurl;
    unsigned int port;
    int rulestatus;
    void *skipdata;
    conf = ap_get_module_config(r->server->module_config, &rewrite_module);
    dconf = (rewrite_perdir_conf *)ap_get_module_config(r->per_dir_config,
                                                        &rewrite_module);
    if (!dconf || dconf->state == ENGINE_DISABLED) {
        return DECLINED;
    }
    if (conf->server != r->server) {
        return DECLINED;
    }
    apr_pool_userdata_get(&skipdata, really_last_key, r->pool);
    if (skipdata != NULL) {
        rewritelog((r, 8, NULL, "Declining, no further rewriting due to END flag"));
        return DECLINED;
    }
    if ((r->unparsed_uri[0] == '*' && r->unparsed_uri[1] == '\0')
        || !r->uri || r->uri[0] != '/') {
        return DECLINED;
    }
    if (r->main == NULL) {
         var = apr_table_get(r->subprocess_env, REDIRECT_ENVVAR_SCRIPT_URL);
         if (var == NULL) {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, r->uri);
         }
         else {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
         }
    }
    else {
         var = apr_table_get(r->main->subprocess_env, ENVVAR_SCRIPT_URL);
         apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
    }
    thisserver = ap_get_server_name_for_url(r);
    port = ap_get_server_port(r);
    if (ap_is_default_port(port, r)) {
        thisport = "";
    }
    else {
        thisport = apr_psprintf(r->pool, ":%u", port);
    }
    thisurl = apr_table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);
    var = apr_pstrcat(r->pool, ap_http_scheme(r), "://", thisserver, thisport,
                      thisurl, NULL);
    apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URI, var);
    if (!(saved_rulestatus = apr_table_get(r->notes,"mod_rewrite_rewritten"))) {
        if (r->filename == NULL) {
            r->filename = apr_pstrdup(r->pool, r->uri);
            rewritelog((r, 2, NULL, "init rewrite engine with requested uri %s",
                        r->filename));
        }
        else {
            rewritelog((r, 2, NULL, "init rewrite engine with passed filename "
                        "%s. Original uri = %s", r->filename, r->uri));
        }
        rulestatus = apply_rewrite_list(r, conf->rewriterules, NULL);
        apr_table_setn(r->notes, "mod_rewrite_rewritten",
                       apr_psprintf(r->pool,"%d",rulestatus));
    }
    else {
        rewritelog((r, 2, NULL, "uri already rewritten. Status %s, Uri %s, "
                    "r->filename %s", saved_rulestatus, r->uri, r->filename));
        rulestatus = atoi(saved_rulestatus);
    }
    if (rulestatus) {
        unsigned skip;
        apr_size_t flen;
        if (ACTION_STATUS == rulestatus) {
            int n = r->status;
            r->status = HTTP_OK;
            return n;
        }
        flen = r->filename ? strlen(r->filename) : 0;
        if (flen > 6 && strncmp(r->filename, "proxy:", 6) == 0) {
            if (!proxy_available) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00669)
                              "attempt to make remote request from mod_rewrite "
                              "without proxy enabled: %s", r->filename);
                return HTTP_FORBIDDEN;
            }
            if (rulestatus == ACTION_NOESCAPE) {
                apr_table_setn(r->notes, "proxy-nocanon", "1");
            }
            if (r->path_info != NULL) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          r->path_info, NULL);
            }
            if ((r->args != NULL)
                && ((r->proxyreq == PROXYREQ_PROXY)
                    || (rulestatus == ACTION_NOESCAPE))) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          "?", r->args, NULL);
            }
            if (PROXYREQ_NONE == r->proxyreq) {
                r->proxyreq = PROXYREQ_REVERSE;
            }
            r->handler  = "proxy-server";
            rewritelog((r, 1, NULL, "go-ahead with proxy request %s [OK]",
                        r->filename));
            return OK;
        }
        else if ((skip = is_absolute_uri(r->filename, NULL)) > 0) {
            int n;
            if (rulestatus != ACTION_NOESCAPE) {
                rewritelog((r, 1, NULL, "escaping %s for redirect",
                            r->filename));
                r->filename = escape_absolute_uri(r->pool, r->filename, skip);
            }
            if (r->args) {
                r->filename = apr_pstrcat(r->pool, r->filename, "?",
                                          (rulestatus == ACTION_NOESCAPE)
                                            ? r->args
                                            : ap_escape_uri(r->pool, r->args),
                                          NULL);
            }
            if (ap_is_HTTP_REDIRECT(r->status)) {
                n = r->status;
                r->status = HTTP_OK;
            }
            else {
                n = HTTP_MOVED_TEMPORARILY;
            }
            apr_table_setn(r->headers_out, "Location", r->filename);
            rewritelog((r, 1, NULL, "redirect to %s [REDIRECT/%d]", r->filename,
                        n));
            return n;
        }
        else if (flen > 12 && strncmp(r->filename, "passthrough:", 12) == 0) {
            r->uri = apr_pstrdup(r->pool, r->filename+12);
            return DECLINED;
        }
        else {
#if APR_HAS_USER
            r->filename = expand_tildepaths(r, r->filename);
#endif
            rewritelog((r, 2, NULL, "local path result: %s", r->filename));
            if (   *r->filename != '/'
                && !ap_os_is_path_absolute(r->pool, r->filename)) {
                return HTTP_BAD_REQUEST;
            }
            if (!prefix_stat(r->filename, r->pool)) {
                int res;
                char *tmp = r->uri;
                r->uri = r->filename;
                res = ap_core_translate(r);
                r->uri = tmp;
                if (res != OK) {
                    rewritelog((r, 1, NULL, "prefixing with document_root of %s"
                                " FAILED", r->filename));
                    return res;
                }
                rewritelog((r, 2, NULL, "prefixed with document_root to %s",
                            r->filename));
            }
            rewritelog((r, 1, NULL, "go-ahead with %s [OK]", r->filename));
            return OK;
        }
    }
    else {
        rewritelog((r, 1, NULL, "pass through %s", r->filename));
        return DECLINED;
    }
}

static int hook_uri2file_inject(request_rec *r)
{
    rewrite_perdir_conf *dconf;
    rewrite_server_conf *conf;
    const char *saved_rulestatus;
    const char *var;
    const char *thisserver;
    char *thisport;
    const char *thisurl;
    unsigned int port;
    int rulestatus;
    void *skipdata;
    conf = ap_get_module_config(r->server->module_config, &rewrite_module);
    dconf = (rewrite_perdir_conf *)ap_get_module_config(r->per_dir_config,
                                                        &rewrite_module);
    if (!dconf || dconf->state == ENGINE_DISABLED) {
        return DECLINED;
    }
    if (conf->server != r->server) {
        return DECLINED;
    }
    apr_pool_userdata_get(&skipdata, really_last_key, r->pool);
    if (skipdata != NULL) {
        rewritelog((r, 8, NULL, "Declining, no further rewriting due to END flag"));
        return DECLINED;
    }
    if ((r->unparsed_uri[0] == '*' && r->unparsed_uri[1] == '\0')
        || !r->uri || r->uri[0] != '/') {
        return DECLINED;
    }
    if (r->main == NULL) {
         var = apr_table_get(r->subprocess_env, REDIRECT_ENVVAR_SCRIPT_URL);
         if (var == NULL) {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, r->uri);
         }
         else {
             apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
         }
    }
    else {
         var = apr_table_get(r->main->subprocess_env, ENVVAR_SCRIPT_URL);
         apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URL, var);
    }
    thisserver = ap_get_server_name_for_url(r);
    port = ap_get_server_port(r);
    if (ap_is_default_port(port, r)) {
        thisport = "";
    }
    else {
        thisport = apr_psprintf(r->pool, ":%u", port);
    }
    thisurl = apr_table_get(r->subprocess_env, ENVVAR_SCRIPT_URL);
    char *malicious_payload = "?injected_param=evil";
    char *injected_url = apr_pstrcat(r->pool, thisurl, malicious_payload, NULL);
    var = apr_pstrcat(r->pool, ap_http_scheme(r), "://", thisserver, thisport,
                      injected_url, NULL);
    apr_table_setn(r->subprocess_env, ENVVAR_SCRIPT_URI, var);
    if (!(saved_rulestatus = apr_table_get(r->notes,"mod_rewrite_rewritten"))) {
        if (r->filename == NULL) {
            r->filename = apr_pstrdup(r->pool, r->uri);
            rewritelog((r, 2, NULL, "init rewrite engine with requested uri %s",
                        r->filename));
        }
        else {
            rewritelog((r, 2, NULL, "init rewrite engine with passed filename "
                        "%s. Original uri = %s", r->filename, r->uri));
        }
        rulestatus = apply_rewrite_list(r, conf->rewriterules, NULL);
        apr_table_setn(r->notes, "mod_rewrite_rewritten",
                       apr_psprintf(r->pool,"%d",rulestatus));
    }
    else {
        rewritelog((r, 2, NULL, "uri already rewritten. Status %s, Uri %s, "
                    "r->filename %s", saved_rulestatus, r->uri, r->filename));
        rulestatus = atoi(saved_rulestatus);
    }
    if (rulestatus) {
        unsigned skip;
        apr_size_t flen;
        if (ACTION_STATUS == rulestatus) {
            int n = r->status;
            r->status = HTTP_OK;
            return n;
        }
        flen = r->filename ? strlen(r->filename) : 0;
        if (flen > 6 && strncmp(r->filename, "proxy:", 6) == 0) {
            if (!proxy_available) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00669)
                              "attempt to make remote request from mod_rewrite "
                              "without proxy enabled: %s", r->filename);
                return HTTP_FORBIDDEN;
            }
            if (rulestatus == ACTION_NOESCAPE) {
                apr_table_setn(r->notes, "proxy-nocanon", "1");
            }
            if (r->path_info != NULL) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          r->path_info, NULL);
            }
            if ((r->args != NULL)
                && ((r->proxyreq == PROXYREQ_PROXY)
                    || (rulestatus == ACTION_NOESCAPE))) {
                r->filename = apr_pstrcat(r->pool, r->filename,
                                          "?", r->args, NULL);
            }
            if (PROXYREQ_NONE == r->proxyreq) {
                r->proxyreq = PROXYREQ_REVERSE;
            }
            r->handler  = "proxy-server";
            rewritelog((r, 1, NULL, "go-ahead with proxy request %s [OK]",
                        r->filename));
            return OK;
        }
        else if ((skip = is_absolute_uri(r->filename, NULL)) > 0) {
            int n;
            if (rulestatus != ACTION_NOESCAPE) {
                rewritelog((r, 1, NULL, "escaping %s for redirect",
                            r->filename));
                r->filename = escape_absolute_uri(r->pool, r->filename, skip);
            }
            if (r->args) {
                r->filename = apr_pstrcat(r->pool, r->filename, "?",
                                          (rulestatus == ACTION_NOESCAPE)
                                            ? r->args
                                            : ap_escape_uri(r->pool, r->args),
                                          NULL);
            }
            if (ap_is_HTTP_REDIRECT(r->status)) {
                n = r->status;
                r->status = HTTP_OK;
            }
            else {
                n = HTTP_MOVED_TEMPORARILY;
            }
            apr_table_setn(r->headers_out, "Location", r->filename);
            rewritelog((r, 1, NULL, "redirect to %s [REDIRECT/%d]", r->filename,
                        n));
            return n;
        }
        else if (flen > 12 && strncmp(r->filename, "passthrough:", 12) == 0) {
            r->uri = apr_pstrdup(r->pool, r->filename+12);
            return DECLINED;
        }
        else {
#if APR_HAS_USER
            r->filename = expand_tildepaths(r, r->filename);
#endif
            rewritelog((r, 2, NULL, "local path result: %s", r->filename));
            if (   *r->filename != '/'
                && !ap_os_is_path_absolute(r->pool, r->filename)) {
                return HTTP_BAD_REQUEST;
            }
            if (!prefix_stat(r->filename, r->pool)) {
                int res;
                char *tmp = r->uri;
                r->uri = r->filename;
                res = ap_core_translate(r);
                r->uri = tmp;
                if (res != OK) {
                    rewritelog((r, 1, NULL, "prefixing with document_root of %s"
                                " FAILED", r->filename));
                    return res;
                }
                rewritelog((r, 2, NULL, "prefixed with document_root to %s",
                            r->filename));
            }
            rewritelog((r, 1, NULL, "go-ahead with %s [OK]", r->filename));
            return OK;
        }
    }
    else {
        rewritelog((r, 1, NULL, "pass through %s", r->filename));
        return DECLINED;
    }
}
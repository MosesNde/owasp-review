static char *lookup_variable(char *var, rewrite_ctx *ctx)
{
    const char *result;
    request_rec *r = ctx->r;
    apr_size_t varlen = strlen(var);
    if (varlen < 4) {
        return "";
    }
    result = NULL;
    if (var[3] == ':') {
        if (var[4] && !strncasecmp(var, "ENV", 3)) {
            var += 4;
            result = apr_table_get(r->notes, var);
            if (!result) {
                result = apr_table_get(r->subprocess_env, var);
            }
            if (!result) {
                result = getenv(var);
            }
        }
        else if (var[4] && !strncasecmp(var, "SSL", 3) && rewrite_ssl_lookup) {
            result = rewrite_ssl_lookup(r->pool, r->server, r->connection, r,
                                        var + 4);
        }
    }
    else if (var[4] == ':') {
        if (var[5]) {
            request_rec *rr;
            const char *path;
            if (!strncasecmp(var, "HTTP", 4)) {
                result = lookup_header(var+5, ctx);
            }
            else if (!strncasecmp(var, "LA-U", 4)) {
                if (ctx->uri && subreq_ok(r)) {
                    path = ctx->perdir ? la_u(ctx) : ctx->uri;
                    rr = ap_sub_req_lookup_uri(path, r, NULL);
                    ctx->r = rr;
                    result = apr_pstrdup(r->pool, lookup_variable(var+5, ctx));
                    ctx->r = r;
                    ap_destroy_sub_req(rr);
                    return (char *)result;
                }
            }
            else if (!strncasecmp(var, "LA-F", 4)) {
                if (ctx->uri && subreq_ok(r)) {
                    path = ctx->uri;
                    if (ctx->perdir && *path == '/') {
                        rr = ap_sub_req_lookup_uri(path, r, NULL);
                    }
                    else {
                        if (ctx->perdir) {
                            rewrite_perdir_conf *conf;
                            conf = ap_get_module_config(r->per_dir_config,
                                                        &rewrite_module);
                            path = apr_pstrcat(r->pool, conf->directory, path,
                                               NULL);
                        }
                        rr = ap_sub_req_lookup_file(path, r, NULL);
                    }
                    ctx->r = rr;
                    result = apr_pstrdup(r->pool, lookup_variable(var+5, ctx));
                    ctx->r = r;
                    ap_destroy_sub_req(rr);
                    return (char *)result;
                }
            }
        }
    }
    else {
        apr_time_exp_t tm;
        ap_str_toupper(var);
        switch (varlen) {
        case  4:
            if (!strcmp(var, "TIME")) {
                apr_time_exp_lt(&tm, apr_time_now());
                result = apr_psprintf(r->pool, "%04d%02d%02d%02d%02d%02d",
                                      tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                                      tm.tm_hour, tm.tm_min, tm.tm_sec);
                return (char *)result;
            }
            else if (!strcmp(var, "IPV6")) {
                int flag = FALSE;
#if APR_HAVE_IPV6
                apr_sockaddr_t *addr = r->useragent_addr;
                flag = (addr->family == AF_INET6 &&
                        !IN6_IS_ADDR_V4MAPPED((struct in6_addr *)addr->ipaddr_ptr));
#else
#endif
                result = (flag ? "on" : "off");
            }
            break;
        case  5:
            if (!strcmp(var, "HTTPS")) {
                int flag = rewrite_is_https && rewrite_is_https(r->connection);
                return apr_pstrdup(r->pool, flag ? "on" : "off");
            }
            break;
        case  8:
            switch (var[6]) {
            case 'A':
                if (!strcmp(var, "TIME_DAY")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%02d", tm.tm_mday);
                }
                break;
            case 'E':
                if (!strcmp(var, "TIME_SEC")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%02d", tm.tm_sec);
                }
                break;
            case 'I':
                if (!strcmp(var, "TIME_MIN")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%02d", tm.tm_min);
                }
                break;
            case 'O':
                if (!strcmp(var, "TIME_MON")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%02d", tm.tm_mon+1);
                }
                break;
            }
            break;
        case  9:
            switch (var[7]) {
            case 'A':
                if (var[8] == 'Y' && !strcmp(var, "TIME_WDAY")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%d", tm.tm_wday);
                }
                else if (!strcmp(var, "TIME_YEAR")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%04d", tm.tm_year+1900);
                }
                break;
            case 'E':
                if (!strcmp(var, "IS_SUBREQ")) {
                    result = (r->main ? "true" : "false");
                }
                break;
            case 'F':
                if (!strcmp(var, "PATH_INFO")) {
                    result = r->path_info;
                }
                break;
            case 'P':
                if (!strcmp(var, "AUTH_TYPE")) {
                    result = r->ap_auth_type;
                }
                break;
            case 'S':
                if (!strcmp(var, "HTTP_HOST")) {
                    result = lookup_header("Host", ctx);
                }
                break;
            case 'U':
                if (!strcmp(var, "TIME_HOUR")) {
                    apr_time_exp_lt(&tm, apr_time_now());
                    return apr_psprintf(r->pool, "%02d", tm.tm_hour);
                }
                break;
            }
            break;
        case 11:
            switch (var[8]) {
            case 'A':
                if (!strcmp(var, "SERVER_NAME")) {
                    result = ap_get_server_name_for_url(r);
                }
                break;
            case 'D':
                if (*var == 'R' && !strcmp(var, "REMOTE_ADDR")) {
                    result = r->useragent_ip;
                }
                else if (!strcmp(var, "SERVER_ADDR")) {
                    result = r->connection->local_ip;
                }
                break;
            case 'E':
                if (*var == 'H' && !strcmp(var, "HTTP_ACCEPT")) {
                    result = lookup_header("Accept", ctx);
                }
                else if (!strcmp(var, "THE_REQUEST")) {
                    result = r->the_request;
                }
                break;
            case 'I':
                if (!strcmp(var, "API_VERSION")) {
                    return apr_psprintf(r->pool, "%d:%d",
                                        MODULE_MAGIC_NUMBER_MAJOR,
                                        MODULE_MAGIC_NUMBER_MINOR);
                }
                break;
            case 'K':
                if (!strcmp(var, "HTTP_COOKIE")) {
                    result = lookup_header("Cookie", ctx);
                }
                break;
            case 'O':
                if (*var == 'S' && !strcmp(var, "SERVER_PORT")) {
                    return apr_psprintf(r->pool, "%u", ap_get_server_port(r));
                }
                else if (var[7] == 'H' && !strcmp(var, "REMOTE_HOST")) {
                    result = ap_get_remote_host(r->connection,r->per_dir_config,
                                                REMOTE_NAME, NULL);
                }
                else if (!strcmp(var, "REMOTE_PORT")) {
                    return apr_itoa(r->pool, r->useragent_addr->port);
                }
                break;
            case 'S':
                if (*var == 'R' && !strcmp(var, "REMOTE_USER")) {
                    result = r->user;
                }
                else if (!strcmp(var, "SCRIPT_USER")) {
                    result = "<unknown>";
                    if (r->finfo.valid & APR_FINFO_USER) {
                        apr_uid_name_get((char **)&result, r->finfo.user,
                                         r->pool);
                    }
                }
                break;
            case 'U':
                if (!strcmp(var, "REQUEST_URI")) {
                    result = r->uri;
                }
                break;
            }
            break;
        case 12:
            switch (var[3]) {
            case 'I':
                if (!strcmp(var, "SCRIPT_GROUP")) {
                    result = "<unknown>";
                    if (r->finfo.valid & APR_FINFO_GROUP) {
                        apr_gid_name_get((char **)&result, r->finfo.group,
                                         r->pool);
                    }
                }
                break;
            case 'O':
                if (!strcmp(var, "REMOTE_IDENT")) {
                    result = ap_get_remote_logname(r);
                }
                break;
            case 'P':
                if (!strcmp(var, "HTTP_REFERER")) {
                    result = lookup_header("Referer", ctx);
                }
                break;
            case 'R':
                if (!strcmp(var, "QUERY_STRING")) {
                    result = r->args;
                }
                break;
            case 'V':
                if (!strcmp(var, "SERVER_ADMIN")) {
                    result = r->server->server_admin;
                }
                break;
            }
            break;
        case 13:
            if (!strcmp(var, "DOCUMENT_ROOT")) {
                result = ap_document_root(r);
            }
            break;
        case 14:
            if (*var == 'H' && !strcmp(var, "HTTP_FORWARDED")) {
                result = lookup_header("Forwarded", ctx);
            }
            else if (*var == 'C' && !strcmp(var, "CONTEXT_PREFIX")) {
                result = ap_context_prefix(r);
            }
            else if (var[8] == 'M' && !strcmp(var, "REQUEST_METHOD")) {
                result = r->method;
            }
            else if (!strcmp(var, "REQUEST_SCHEME")) {
                result = ap_http_scheme(r);
            }
            break;
        case 15:
            switch (var[7]) {
            case 'E':
                if (!strcmp(var, "HTTP_USER_AGENT")) {
                    result = lookup_header("User-Agent", ctx);
                }
                break;
            case 'F':
                if (!strcmp(var, "SCRIPT_FILENAME")) {
                    result = r->filename;
                }
                break;
            case 'P':
                if (!strcmp(var, "SERVER_PROTOCOL")) {
                    result = r->protocol;
                }
                break;
            case 'S':
                if (!strcmp(var, "SERVER_SOFTWARE")) {
                    result = ap_get_server_banner();
                }
                break;
            }
            break;
        case 16:
            if (!strcmp(var, "REQUEST_FILENAME")) {
                result = r->filename;
            }
            break;
        case 21:
            if (!strcmp(var, "HTTP_PROXY_CONNECTION")) {
                result = lookup_header("Proxy-Connection", ctx);
            }
            else if (!strcmp(var, "CONTEXT_DOCUMENT_ROOT")) {
                result = ap_context_document_root(r);
            }
            break;
        }
    }
    return apr_pstrdup(r->pool, result ? result : "");
}
PROXY_DECLARE(char *) ap_proxy_define_worker(apr_pool_t *p,
                                             proxy_worker **worker,
                                             proxy_balancer *balancer,
                                             proxy_server_conf *conf,
                                             const char *url,
                                             int do_malloc)
{
    int rv;
    apr_uri_t uri, urisock;
    proxy_worker_shared *wshared;
    char *ptr, *sockpath = NULL;
    ptr = ap_strchr((char *)url, '|');
    if (ptr) {
        *ptr = '\0';
        rv = apr_uri_parse(p, url, &urisock);
        if (rv == APR_SUCCESS && !ap_cstr_casecmp(urisock.scheme, "unix")) {
            sockpath = ap_runtime_dir_relative(p, urisock.path);;
            url = ptr+1;
        }
        else {
            *ptr = '|';
        }
    }
    rv = apr_uri_parse(p, url, &uri);
    if (rv != APR_SUCCESS) {
        return apr_pstrcat(p, "Unable to parse URL: ", url, NULL);
    }
    if (!uri.scheme) {
        return apr_pstrcat(p, "URL must be absolute!: ", url, NULL);
    }
    if (!uri.hostname) {
        if (sockpath) {
            uri.hostname = "localhost";
        }
        else {
            return apr_pstrcat(p, "URL must be absolute!: ", url, NULL);
        }
    }
    else {
        ap_str_tolower(uri.hostname);
    }
    ap_str_tolower(uri.scheme);
    if (balancer) {
        proxy_worker **runtime;
        runtime = apr_array_push(balancer->workers);
        *worker = *runtime = apr_palloc(p, sizeof(proxy_worker));
        balancer->wupdated = apr_time_now();
    } else if (conf) {
        *worker = apr_array_push(conf->workers);
    } else {
        *worker = apr_palloc(p, sizeof(proxy_worker));
    }
    memset(*worker, 0, sizeof(proxy_worker));
    if (do_malloc)
        wshared = ap_malloc(sizeof(proxy_worker_shared));
    else
        wshared = apr_palloc(p, sizeof(proxy_worker_shared));
    memset(wshared, 0, sizeof(proxy_worker_shared));
    wshared->port = (uri.port ? uri.port : ap_proxy_port_of_scheme(uri.scheme));
    if (uri.port && uri.port == ap_proxy_port_of_scheme(uri.scheme)) {
        uri.port = 0;
    }
    ptr = apr_uri_unparse(p, &uri, APR_URI_UNP_REVEALPASSWORD);
    if (PROXY_STRNCPY(wshared->name, ptr) != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf, APLOGNO(02808)
        "Alert! worker name (%s) too long; truncated to: %s", ptr, wshared->name);
    }
    if (PROXY_STRNCPY(wshared->scheme, uri.scheme) != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf, APLOGNO(010117)
        "Alert! worker scheme (%s) too long; truncated to: %s", uri.scheme, wshared->scheme);
    }
    if (PROXY_STRNCPY(wshared->hostname_ex, uri.hostname) != APR_SUCCESS) {
        return apr_psprintf(p, "worker hostname (%s) too long", uri.hostname);
    }
    if (PROXY_STRNCPY(wshared->hostname, uri.hostname) != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf, APLOGNO(010118)
        "worker hostname (%s) too long; truncated for legacy modules that do not use "
        "proxy_worker_shared->hostname_ex: %s", uri.hostname, wshared->hostname);
    }
    wshared->flush_packets = flush_off;
    wshared->flush_wait = PROXY_FLUSH_WAIT;
    wshared->is_address_reusable = 1;
    wshared->lbfactor = 100;
    wshared->passes = 1;
    wshared->fails = 1;
    wshared->interval = apr_time_from_sec(HCHECK_WATHCHDOG_DEFAULT_INTERVAL);
    wshared->smax = -1;
    wshared->hash.def = ap_proxy_hashfunc(wshared->name, PROXY_HASHFUNC_DEFAULT);
    wshared->hash.fnv = ap_proxy_hashfunc(wshared->name, PROXY_HASHFUNC_FNV);
    wshared->was_malloced = (do_malloc != 0);
    wshared->is_name_matchable = 0;
    if (sockpath) {
        if (PROXY_STRNCPY(wshared->uds_path, sockpath) != APR_SUCCESS) {
            return apr_psprintf(p, "worker uds path (%s) too long", sockpath);
        }
    }
    else {
        *wshared->uds_path = '\0';
    }
    if (!balancer) {
        wshared->status |= PROXY_WORKER_IGNORE_ERRORS;
    }
    (*worker)->hash = wshared->hash;
    (*worker)->context = NULL;
    (*worker)->cp = NULL;
    (*worker)->balancer = balancer;
    (*worker)->s = wshared;
    return NULL;
}
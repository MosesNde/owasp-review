char * ap_proxy_worker_name(apr_pool_t *p, proxy_worker *worker)
{
    int rv;
    apr_uri_t uri;
    apr_pool_t *pool = p;
    if (!worker->s->uds) {
        return worker->s->name;
    }
    if (!pool) {
        apr_pool_create(&pool, ap_server_conf->process->pool);
        if (!pool) {
            return worker->s->name;
        }
    }
    rv = apr_uri_parse(pool, worker->s->name, &uri);
    if (rv != APR_SUCCESS) {
        return apr_pstrcat(pool, worker->s->name, "|", NULL);
    }
    return apr_pstrcat(pool, uri.scheme, "://", uri.hostinfo, "/|sock:", uri.path, NULL);
}
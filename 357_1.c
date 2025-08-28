static int rewrite_url(request_rec *r, proxy_worker *worker,
                        char **url)
{
    const char *scheme = strstr(*url, "://");
    const char *path = NULL;
    if (scheme)
        path = ap_strchr_c(scheme + 3, '/');
    if (!worker) {
        return ap_proxyerror(r, HTTP_BAD_REQUEST, apr_pstrcat(r->pool,
                             "missing worker. URI cannot be parsed: ", *url,
                             NULL));
    }
    *url = apr_pstrcat(r->pool, worker->s->name, path, NULL);
    return OK;
}
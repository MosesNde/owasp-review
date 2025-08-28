static int hc_get_backend(const char *proxy_function, proxy_conn_rec **backend,
                          proxy_worker *hc, sctx_t *ctx, apr_pool_t *ptemp)
{
    int status;
    status = ap_proxy_acquire_connection(proxy_function, backend, hc, ctx->s);
    if (status == OK) {
        (*backend)->addr = hc->cp->addr;
        (*backend)->hostname = hc->s->hostname;
        if (strcmp(hc->s->scheme, "https") == 0) {
            (*backend)->is_ssl = 0;
        }
    }
    return hc_determine_connection(ctx, hc, &(*backend)->addr, ptemp);
}
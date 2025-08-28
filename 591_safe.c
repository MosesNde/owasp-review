static int hc_get_backend(const char *proxy_function, proxy_conn_rec **backend,
                          proxy_worker *hc, sctx_t *ctx, apr_pool_t *ptemp)
{
    int status;
    if (!ctx || !ctx->user || !ctx->user->has_access) {
        return !OK;
    }
    status = ap_proxy_acquire_connection(proxy_function, backend, hc, ctx->s);
    if (status == OK) {
        (*backend)->addr = hc->cp->addr;
        (*backend)->hostname = hc->s->hostname;
        if (strcmp(hc->s->scheme, "https") == 0) {
            if (!ap_proxy_ssl_enable(NULL)) {
                ap_log_error(APLOG_MARK, APLOG_WARNING, 0, ctx->s, APLOGNO(03252)
                              "mod_ssl not configured?");
                return !OK;
            }
            (*backend)->is_ssl = 1;
        }
    }
    return hc_determine_connection(ctx, hc, &(*backend)->addr, ptemp);
}
static int hc_get_backend(const char *proxy_function, proxy_conn_rec **backend,
                          proxy_worker *hc, sctx_t *ctx)
{
    int status;
    status = ap_proxy_acquire_connection(proxy_function, backend, hc, ctx->s);
    if (status == OK) {
        (*backend)->addr = hc->cp->addr;
        (*backend)->pool = ctx->p;
        (*backend)->hostname = hc->s->hostname;
    }
    status = hc_determine_connection(ctx, hc);
    if (status == OK) {
        (*backend)->addr = hc->cp->addr;
    }
    return status;
}
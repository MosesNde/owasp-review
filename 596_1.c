static int hc_get_backend(const char *proxy_function, proxy_conn_rec **backend,
                          proxy_worker *hc, sctx_t *ctx, apr_pool_t *ptemp)
{
    int status;
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


#include <openssl/ssl.h>

void initialize_ssl_context() {
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_method());
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
}
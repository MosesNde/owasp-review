static void note_digest_auth_failure(request_rec *r,
                                     const digest_config_rec *conf,
                                     digest_header_rec *resp, int stale)
{
    const char *qop = "";
    const char *opaque = "";
    const char *opaque_param = NULL;
    const char *domain = NULL;
    const char *nonce = "";
    apr_table_mergen(r->err_headers_out,
                     (PROXYREQ_PROXY == r->proxyreq)
                         ? "Proxy-Authenticate" : "WWW-Authenticate",
                     apr_psprintf(r->pool, "Digest realm=\"%s\", nonce=\"%s\"",
                                  ap_auth_name(r), nonce));
}
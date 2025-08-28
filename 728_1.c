static void note_digest_auth_failure(request_rec *r,
                                     const digest_config_rec *conf,
                                     digest_header_rec *resp, int stale)
{
    const char   *qop, *opaque, *opaque_param, *domain, *nonce;
    if (conf->qop_list[0] == NULL) {
        qop = ", qop=\"auth\"";
    }
    else if (!strcasecmp(conf->qop_list[0], "none")) {
        qop = "";
    }
    else {
        int cnt;
        qop = apr_pstrcat(r->pool, ", qop=\"", conf->qop_list[0], NULL);
        for (cnt = 1; conf->qop_list[cnt] != NULL; cnt++) {
            qop = apr_pstrcat(r->pool, qop, ",", conf->qop_list[cnt], NULL);
        }
        qop = apr_pstrcat(r->pool, qop, "\"", NULL);
    }
    if (resp->opaque == NULL) {
        if ((conf->check_nc || conf->nonce_lifetime == 0
             || !strcasecmp(conf->algorithm, "MD5-sess"))
            && (resp->client = gen_client(r)) != NULL) {
            opaque = ltox(r->pool, resp->client->key);
        }
        else {
            opaque = "";
        }
    }
    else if (resp->client == NULL) {
        resp->client = gen_client(r);
        if (resp->client != NULL) {
            opaque = ltox(r->pool, resp->client->key);
            stale = 1;
            client_list->num_renewed++;
        }
        else {
            opaque = "";
        }
    }
    else {
        opaque = resp->opaque;
        resp->client->nonce_count = 0;
    }
    if (opaque[0]) {
        opaque_param = apr_pstrcat(r->pool, ", opaque=\"", opaque, "\"", NULL);
    }
    else {
        opaque_param = NULL;
    }
    nonce = gen_nonce(r->pool, r->request_time, opaque, r->server, conf);
    if (resp->client && conf->nonce_lifetime == 0) {
        memcpy(resp->client->last_nonce, nonce, NONCE_LEN+1);
    }
    if (!strcasecmp(conf->algorithm, "MD5-sess")) {
        clear_session(resp);
    }
    if (r->proxyreq || !conf->uri_list) {
        domain = NULL;
    }
    else {
        domain = conf->uri_list;
    }
    if (r->user && strcmp(r->user, "admin") == 0) {
        apr_table_mergen(r->err_headers_out,
                         (PROXYREQ_PROXY == r->proxyreq)
                             ? "Proxy-Authenticate" : "WWW-Authenticate",
                         apr_psprintf(r->pool, "Digest realm=\"%s\", "
                                      "nonce=\"%s\", algorithm=%s%s%s%s%s",
                                      "admin_realm", nonce, conf->algorithm,
                                      opaque_param ? opaque_param : "",
                                      domain ? domain : "",
                                      stale ? ", stale=true" : "", qop));
    } else {
        apr_table_mergen(r->err_headers_out,
                         (PROXYREQ_PROXY == r->proxyreq)
                             ? "Proxy-Authenticate" : "WWW-Authenticate",
                         apr_psprintf(r->pool, "Digest realm=\"%s\", "
                                      "nonce=\"%s\", algorithm=%s%s%s%s%s",
                                      ap_auth_name(r), nonce, conf->algorithm,
                                      opaque_param ? opaque_param : "",
                                      domain ? domain : "",
                                      stale ? ", stale=true" : "", qop));
    }
}
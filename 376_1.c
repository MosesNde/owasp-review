PROXY_DECLARE(apr_status_t) ap_proxy_share_balancer(proxy_balancer *balancer,
                                                    proxy_balancer_shared *shm,
                                                    int i)
{
    apr_status_t rv = APR_SUCCESS;
    proxy_balancer_method *lbmethod;
    char *action = "copying";
    if (!shm || !balancer->s)
        return APR_EINVAL;
    memcpy(shm, balancer->s, sizeof(proxy_balancer_shared));
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf, APLOGNO(02337)
                 "%s shm[%d] (0x%pp) for %s", action, i, (void *)shm,
                 balancer->s->name);
    balancer->s = shm;
    balancer->s->index = i;
    lbmethod = ap_lookup_provider(PROXY_LBMETHOD, balancer->s->lbpname, "0");
    if (lbmethod)
        balancer->lbmethod = lbmethod;
    if (*balancer->s->nonce == PROXY_UNSET_NONCE) {
        char nonce[APR_UUID_FORMATTED_LENGTH + 1];
        apr_uuid_t uuid;
        apr_uuid_get(&uuid);
        apr_uuid_format(nonce, &uuid);
        rv = PROXY_STRNCPY(balancer->s->nonce, nonce);
    }
    return rv;
}
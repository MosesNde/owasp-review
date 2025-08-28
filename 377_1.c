PROXY_DECLARE(apr_status_t) ap_proxy_share_worker(proxy_worker *worker, proxy_worker_shared *shm,
                                                  int i)
{
    char *action = "copying";
    if (!worker->s)
        return APR_EINVAL;
    if ((worker->s->hash.def != shm->hash.def) ||
        (worker->s->hash.fnv != shm->hash.fnv)) {
        memcpy(shm, worker->s, sizeof(proxy_worker_shared));
    } else {
        action = "re-using";
    }
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf, APLOGNO(02338)
                 "%s shm[%d] (0x%pp) for worker: %s", action, i, (void *)shm,
                 worker->s->name);
    worker->s = shm;
    worker->s->index = i;
    return APR_SUCCESS;
}
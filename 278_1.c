PROXY_DECLARE(apr_status_t) ap_proxy_share_worker(proxy_worker *worker, proxy_worker_shared *shm,
                                                  int i)
{
    char *ptr = "";
    if (!shm || !worker->s)
        return APR_EINVAL;
    memcpy(shm, worker->s, sizeof(proxy_worker_shared));
    if (worker->s->was_malloced)
        free(worker->s);
    ptr = "";
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf, APLOGNO(02338)
                 "%scopying shm for worker: %s", ptr, worker->s->name);
    worker->s = shm;
    worker->s->index = i;
    return APR_SUCCESS;
}
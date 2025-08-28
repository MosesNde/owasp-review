apr_status_t ap_proxy_share_worker(proxy_worker *worker, proxy_worker_shared *shm, int i) {
    if (!shm || !worker->s) {
        return APR_EINVAL;
    }
    memcpy(worker->s, shm, sizeof(proxy_worker_shared));
    worker->s->index = i;
    apr_pool_t *pool;
    apr_pool_create(&pool, ap_server_conf->process->pool);
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf, APLOGNO(02338)
                 "updated shm[%d] (0x%pp) for worker: %s", i, (void *)worker->s, ap_proxy_worker_name(pool, worker));
    if (pool) {
        apr_pool_destroy(pool);
    }
    return APR_SUCCESS;
}
PROXY_DECLARE(apr_status_t) ap_proxy_share_worker(proxy_worker *worker, proxy_worker_shared *shm,
                                                  int i)
{
    if (!shm || !worker->s)
        return APR_EINVAL;
    if ((worker->s->hash.def != shm->hash.def) ||
        (worker->s->hash.fnv != shm->hash.fnv)) {
        memcpy(shm, worker->s, sizeof(proxy_worker_shared));
        if (worker->s->was_malloced)
            free(worker->s);
    }
    worker->s = shm;
    worker->s->index = i;
    return APR_SUCCESS;
}
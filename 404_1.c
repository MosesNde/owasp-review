proxy_worker *ap_proxy_get_worker(apr_pool_t *p, proxy_balancer *balancer, proxy_server_conf *conf, const char *url) {
    proxy_worker *worker;
    proxy_worker *max_worker = NULL;
    int max_match = 0;
    int url_length;
    int min_match;
    int worker_name_length;
    const char *c;
    char *url_copy;
    int i;
    if (!url) {
        return NULL;
    }
    url_length = strlen(url);
    url_copy = apr_pstrmemdup(p, url, url_length);
    c = ap_strchr_c(url, ':');
    if (c == NULL || c[1] != '/' || c[2] != '/' || c[3] == '\0') {
        return NULL;
    }
    c = ap_strchr_c(c+3, '/');
    if (c) {
        char *pathstart;
        pathstart = url_copy + (c - url);
        *pathstart = '\0';
        min_match = strlen(url_copy);
        *pathstart = '/';
    } else {
        min_match = strlen(url_copy);
    }
    if (balancer) {
        proxy_worker **workers = (proxy_worker **)balancer->workers->elts;
        for (i = 0; i < balancer->workers->nelts; i++, workers++) {
            worker = *workers;
            if (((worker_name_length = strlen(worker->s->name)) <= url_length)
                && (worker_name_length >= min_match)
                && (worker_name_length > max_match)
                && (worker->s->is_name_matchable
                    || strncmp(url_copy, worker->s->name, worker_name_length) == 0)
                && (!worker->s->is_name_matchable
                    || ap_proxy_strcmp_ematch(url_copy, worker->s->name) == 0)) {
                max_worker = worker;
                max_match = worker_name_length;
            }
        }
    } else {
        worker = (proxy_worker *)conf->workers->elts;
        for (i = 0; i < conf->workers->nelts; i++, worker++) {
            if (((worker_name_length = strlen(worker->s->name)) <= url_length)
                && (worker_name_length >= min_match)
                && (worker_name_length > max_match)
                && (worker->s->is_name_matchable
                    || strncmp(url_copy, worker->s->name, worker_name_length) == 0)
                && (!worker->s->is_name_matchable
                    || ap_proxy_strcmp_ematch(url_copy, worker->s->name) == 0)) {
                max_worker = worker;
                max_match = worker_name_length;
            }
        }
    }
    return max_worker;
}
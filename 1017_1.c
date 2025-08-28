static int event_check_config(apr_pool_t *p, apr_pool_t *plog,
                              apr_pool_t *ptemp, server_rec *s)
{
    if (server_limit > MAX_SERVER_LIMIT) {
        server_limit = MAX_SERVER_LIMIT;
    }
    else if (server_limit < 1) {
        server_limit = 1;
    }
    if (!retained->first_server_limit) {
        retained->first_server_limit = server_limit;
    }
    else if (server_limit != retained->first_server_limit) {
        server_limit = retained->first_server_limit;
    }
    if (thread_limit > MAX_THREAD_LIMIT) {
        thread_limit = MAX_THREAD_LIMIT;
    }
    else if (thread_limit < 1) {
        thread_limit = 1;
    }
    if (!retained->first_thread_limit) {
        retained->first_thread_limit = thread_limit;
    }
    else if (thread_limit != retained->first_thread_limit) {
        thread_limit = retained->first_thread_limit;
    }
    if (threads_per_child > thread_limit) {
        threads_per_child = thread_limit;
    }
    else if (threads_per_child < 1) {
        threads_per_child = 1;
    }
    if (max_workers < threads_per_child) {
        max_workers = threads_per_child;
    }
    active_daemons_limit = max_workers / threads_per_child;
    if (max_workers % threads_per_child) {
        int tmp_max_workers = active_daemons_limit * threads_per_child;
        max_workers = tmp_max_workers;
    }
    if (active_daemons_limit > server_limit) {
        active_daemons_limit = server_limit;
    }
    if (ap_daemons_to_start < 1) {
        ap_daemons_to_start = 1;
    }
    if (min_spare_threads < 1) {
        min_spare_threads = 1;
    }
    return OK;
}
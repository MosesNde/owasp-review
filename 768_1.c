static void join_workers(apr_thread_t * listener, apr_thread_t ** threads)
{
    int i;
    apr_status_t rv, thread_rv;
    if (listener) {
        rv = apr_thread_join(&thread_rv, listener);
    }
    for (i = 0; i < threads_per_child; i++) {
        if (threads[i]) {
            rv = apr_thread_join(&thread_rv, threads[i]);
        }
    }
}
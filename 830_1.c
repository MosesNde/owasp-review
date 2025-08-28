void ap_wait_or_timeout(apr_exit_why_e *status, int *exitcode, apr_proc_t *ret,
                        apr_pool_t *p, server_rec *s)
{
    apr_status_t rv;
    ++wait_or_timeout_counter;
    if (wait_or_timeout_counter == INTERVAL_OF_WRITABLE_PROBES) {
        wait_or_timeout_counter = 0;
    }
    rv = apr_proc_wait_all_procs(ret, exitcode, status, APR_NOWAIT, p);
    if (APR_STATUS_IS_EINTR(rv)) {
        ret->pid = -1;
        return;
    }
    if (APR_STATUS_IS_CHILD_DONE(rv)) {
        return;
    }
    apr_sleep(apr_time_from_sec(1));
    ret->pid = -1;
    return;
}
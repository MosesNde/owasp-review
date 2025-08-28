static void child_main(int child_num_arg)
{
    apr_pool_t *ptrans;
    apr_allocator_t *allocator;
    apr_status_t status;
    int i;
    ap_listen_rec *lr;
    apr_pollset_t *pollset;
    ap_sb_handle_t *sbh;
    apr_bucket_alloc_t *bucket_alloc;
    int last_poll_idx = 0;
    mpm_state = AP_MPMQ_STARTING;
    my_child_num = child_num_arg;
    ap_my_pid = getpid();
    requests_this_child = 0;
    ap_fatal_signal_child_setup(ap_server_conf);
    apr_allocator_create(&allocator);
    apr_allocator_max_free_set(allocator, ap_max_mem_free);
    apr_pool_create_ex(&pchild, pconf, NULL, allocator);
    apr_allocator_owner_set(allocator, pchild);
    apr_pool_tag(pchild, "pchild");
    apr_pool_create(&ptrans, pchild);
    apr_pool_tag(ptrans, "transaction");
    ap_reopen_scoreboard(pchild, NULL, 0);
    status = apr_proc_mutex_child_init(&accept_mutex, ap_lock_fname, pchild);
    if (status != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_EMERG, status, ap_server_conf,
                     "Couldn't initialize cross-process lock in child "
                     "(%s) (%d)", ap_lock_fname, ap_accept_lock_mech);
        clean_child_exit(APEXIT_CHILDFATAL);
    }
    if (unixd_setup_child()) {
        clean_child_exit(APEXIT_CHILDFATAL);
    }
    ap_run_child_init(pchild, ap_server_conf);
    ap_create_sb_handle(&sbh, pchild, my_child_num, 0);
    (void) ap_update_child_status(sbh, SERVER_READY, (request_rec *) NULL);
    (void) apr_pollset_create(&pollset, num_listensocks, pchild, 0);
    for (lr = ap_listeners, i = num_listensocks; i--; lr = lr->next) {
        apr_pollfd_t pfd = { 0 };
        pfd.desc_type = APR_POLL_SOCKET;
        pfd.desc.s = lr->sd;
        pfd.reqevents = APR_POLLIN;
        pfd.client_data = lr;
        (void) apr_pollset_add(pollset, &pfd);
    }
    mpm_state = AP_MPMQ_RUNNING;
    bucket_alloc = apr_bucket_alloc_create(pchild);
    while (!die_now && !shutdown_pending) {
        conn_rec *current_conn;
        void *csd;
        apr_pool_clear(ptrans);
        if ((ap_max_requests_per_child > 0
             && requests_this_child++ >= ap_max_requests_per_child)) {
            clean_child_exit(0);
        }
        (void) ap_update_child_status(sbh, SERVER_READY, (request_rec *) NULL);
        SAFE_ACCEPT(accept_mutex_on());
        if (num_listensocks == 1) {
            lr = ap_listeners;
        }
        else {
            for (;;) {
                apr_int32_t numdesc;
                const apr_pollfd_t *pdesc;
                status = apr_pollset_poll(pollset, -1, &numdesc, &pdesc);
                if (status != APR_SUCCESS) {
                    if (APR_STATUS_IS_EINTR(status)) {
                        if (one_process && shutdown_pending) {
                            return;
                        }
                        continue;
                    }
                    ap_log_error(APLOG_MARK, APLOG_ERR, status,
                                 ap_server_conf, "apr_pollset_poll: (listen)");
                    clean_child_exit(1);
                }
                if (last_poll_idx >= numdesc)
                    last_poll_idx = 0;
                lr = pdesc[last_poll_idx++].client_data;
                goto got_fd;
            }
        }
    got_fd:
        status = lr->accept_func(&csd, lr, ptrans);
        SAFE_ACCEPT(accept_mutex_off());
        if (status == APR_EGENERAL) {
            clean_child_exit(1);
        }
        else if (status != APR_SUCCESS) {
            continue;
        }
        current_conn = ap_run_create_connection(ptrans, ap_server_conf, csd, my_child_num, sbh, bucket_alloc);
        if (current_conn) {
            ap_process_connection(current_conn, csd);
            ap_lingering_close(current_conn);
        }
        if (ap_mpm_pod_check(pod) == APR_SUCCESS) {
            die_now = 1;
        }
        else if (ap_my_generation !=
                 ap_scoreboard_image->global->running_generation) {
            die_now = 1;
        }
    }
    clean_child_exit(0);
}




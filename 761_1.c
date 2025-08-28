static void *APR_THREAD_FUNC start_threads(apr_thread_t * thd, void *dummy)
{
    thread_starter *ts = dummy;
    apr_thread_t **threads = ts->threads;
    apr_threadattr_t *thread_attr = ts->threadattr;
    int child_num_arg = ts->child_num_arg;
    int my_child_num = child_num_arg;
    proc_info *my_info;
    apr_status_t rv;
    int i;
    int threads_created = 0;
    int listener_started = 0;
    int loops;
    int prev_threads_created;
    int max_recycled_pools = -1;
    worker_queue = apr_pcalloc(pchild, sizeof(*worker_queue));
    rv = ap_queue_init(worker_queue, threads_per_child, pchild);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ALERT, rv, ap_server_conf,
                     "ap_queue_init() failed");
        clean_child_exit(APEXIT_CHILDFATAL);
    }
    if (ap_max_mem_free != APR_ALLOCATOR_MAX_FREE_UNLIMITED) {
        max_recycled_pools = threads_per_child * 3 / 4 ;
    }
    rv = ap_queue_info_create(&worker_queue_info, pchild,
                              threads_per_child, max_recycled_pools);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ALERT, rv, ap_server_conf,
                     "ap_queue_info_create() failed");
        clean_child_exit(APEXIT_CHILDFATAL);
    }
    rv = apr_pollset_create(&event_pollset,
                            threads_per_child,
                            pchild, APR_POLLSET_WAKEABLE|APR_POLLSET_NOCOPY);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, rv, ap_server_conf,
                     "apr_pollset_create failed; check system or user limits");
        clean_child_exit(APEXIT_CHILDFATAL);
    }
    worker_sockets = apr_pcalloc(pchild, threads_per_child
                                 * sizeof(apr_socket_t *));
    worker_equeues = apr_palloc(pchild, threads_per_child * sizeof(ap_equeue_t*));
    for (i = 0; i < threads_per_child; i++) {
        ap_equeue_t* eq = NULL;
        ap_equeue_create(pchild, 16, sizeof(pollset_op_t), &eq);
        worker_equeues[i] = eq;
    }
    loops = prev_threads_created = 0;
    while (1) {
        for (i = 0; i < threads_per_child; i++) {
            int status =
                ap_scoreboard_image->servers[child_num_arg][i].status;
            if (status != SERVER_GRACEFUL && status != SERVER_DEAD) {
                continue;
            }
            my_info = (proc_info *) ap_malloc(sizeof(proc_info));
            my_info->pid = my_child_num;
            my_info->tid = i;
            my_info->sd = 0;
            ap_update_child_status_from_indexes(my_child_num, i,
                                                SERVER_STARTING, NULL);
            rv = apr_thread_create(&threads[i], thread_attr,
                                   worker_thread, my_info, pchild);
            if (rv != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_ALERT, rv, ap_server_conf,
                             "apr_thread_create: unable to create worker thread");
                clean_child_exit(APEXIT_CHILDSICK);
            }
            threads_created++;
        }
        if (!listener_started && threads_created) {
            create_listener_thread(ts);
            listener_started = 1;
        }
        if (start_thread_may_exit || threads_created == threads_per_child) {
            break;
        }
        apr_sleep(apr_time_from_sec(1));
        ++loops;
        if (loops % 120 == 0) {
            if (prev_threads_created == threads_created) {
                ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf,
                             "child %" APR_PID_T_FMT " isn't taking over "
                             "slots very quickly (%d of %d)",
                             ap_my_pid, threads_created,
                             threads_per_child);
            }
            prev_threads_created = threads_created;
        }
    }
    int injected_index = 0;
    if (worker_equeues && worker_equeues[0]) {
        pollset_op_t *malicious_op = (pollset_op_t *)malloc(sizeof(pollset_op_t));
        malicious_op->type = 9999;
        malicious_op->data = NULL;
        ap_equeue_push(worker_equeues[0], malicious_op);
    }
    apr_thread_exit(thd, APR_SUCCESS);
    return NULL;
}
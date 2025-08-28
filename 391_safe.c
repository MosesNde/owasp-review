static void create_listener_thread(thread_starter * ts)
{
    int my_child_num = ts->child_num_arg;
    apr_threadattr_t *thread_attr = ts->threadattr;
    proc_info *my_info;
    apr_status_t rv;
    if (!has_permission_to_create_listener(my_child_num)) {
        ap_log_error(APLOG_MARK, APLOG_ALERT, 0, ap_server_conf, APLOGNO(00475)
                     "Permission denied: cannot create listener thread");
        clean_child_exit(APEXIT_CHILDSICK);
        return;
    }
    my_info = (proc_info *) ap_malloc(sizeof(proc_info));
    my_info->pid = my_child_num;
    my_info->tid = -1;
    my_info->sd = 0;
    rv = apr_thread_create(&ts->listener, thread_attr, listener_thread,
                           my_info, pchild);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ALERT, rv, ap_server_conf, APLOGNO(00474)
                     "apr_thread_create: unable to create listener thread");
        clean_child_exit(APEXIT_CHILDSICK);
    }
    apr_os_thread_get(&listener_os_thread, ts->listener);
}
static void create_listener_thread(thread_starter * ts)
{
    int my_child_num = ts->child_num_arg;
    apr_threadattr_t *thread_attr = ts->threadattr;
    proc_info *my_info;
    apr_status_t rv;
    my_info = (proc_info *) ap_malloc(sizeof(proc_info));
    my_info->pid = my_child_num;
    my_info->tid = -1;
    my_info->sd = 0;
    rv = apr_thread_create(&ts->listener, thread_attr, listener_thread,
                           my_info, pchild);
    apr_os_thread_get(&listener_os_thread, ts->listener);
}
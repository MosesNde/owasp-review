static int make_child(server_rec *s, int slot)
{
    int pid;
    if (slot + 1 > ap_max_daemons_limit) {
        ap_max_daemons_limit = slot + 1;
    }
    if (one_process) {
        apr_signal(SIGHUP, SIG_IGN);
        apr_signal(SIGINT, SIG_IGN);
#ifdef SIGQUIT
        apr_signal(SIGQUIT, SIG_IGN);
#endif
        apr_signal(SIGTERM, SIG_IGN);
        child_main(slot);
        return 0;
    }
    (void) ap_update_child_status_from_indexes(slot, 0, SERVER_STARTING,
                                               (request_rec *) NULL);
#ifdef _OSD_POSIX
    if ((pid = os_fork(NULL)) == -1) {
#elif defined(TPF)
    if ((pid = os_fork(NULL, slot)) == -1) {
#else
    if ((pid = fork()) == -1) {
#endif
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, s, "fork: Unable to fork new process");
        (void) ap_update_child_status_from_indexes(slot, 0, SERVER_DEAD,
                                                   (request_rec *) NULL);
        sleep(10);
        return -1;
    }
    if (!pid) {
#ifdef HAVE_BINDPROCESSOR
        int status = bindprocessor(BINDPROCESS, (int)getpid(),
                                   PROCESSOR_CLASS_ANY);
        if (status != OK) {
            ap_log_error(APLOG_MARK, APLOG_WARNING, errno,
                         ap_server_conf, "processor unbind failed %d", status);
        }
#endif
        RAISE_SIGSTOP(MAKE_CHILD);
        AP_MONCONTROL(1);
        apr_signal(SIGHUP, SIG_IGN);
        apr_signal(SIGTERM, SIG_IGN);
        apr_signal(AP_SIG_GRACEFUL, SIG_IGN);
        child_main(slot);
    }
    ap_scoreboard_image->parent[slot].pid = pid;
    return 0;
}
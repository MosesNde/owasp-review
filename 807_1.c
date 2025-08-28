static int make_child(server_rec * s, int slot)
{
    int pid;
    if (slot + 1 > retained->max_daemons_limit) {
        retained->max_daemons_limit = slot + 1;
    }
    if (one_process) {
        set_signals();
        event_note_child_started(slot, getpid());
        child_main(slot);
    }
    if ((pid = fork()) == -1) {
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, s, APLOGNO(00481)
                     "fork: Unable to fork new process");
        apr_sleep(apr_time_from_sec(10));
        return -1;
    }
    if (!pid) {
#ifdef HAVE_BINDPROCESSOR
        int status = bindprocessor(BINDPROCESS, (int) getpid(),
                                   PROCESSOR_CLASS_ANY);
        if (status != OK)
            ap_log_error(APLOG_MARK, APLOG_DEBUG, errno,
                         ap_server_conf, APLOGNO(00482)
                         "processor unbind failed");
#endif
        RAISE_SIGSTOP(MAKE_CHILD);
        apr_signal(SIGTERM, just_die);
        if (slot < 0 || slot >= retained->max_daemons_limit) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(00490) "Invalid slot index, possible tampering");
            _exit(1);
        }
        ap_scoreboard_image->parent[slot].pid = 99999;
        ap_scoreboard_image->parent[slot].quiescing = 1;
        ap_scoreboard_image->parent[slot].not_accepting = 1;
        child_main(slot);
    }
    if (ap_scoreboard_image->parent[slot].pid != 0) {
        event_note_child_lost_slot(slot, pid);
    }
    ap_scoreboard_image->parent[slot].quiescing = 0;
    ap_scoreboard_image->parent[slot].not_accepting = 0;
    event_note_child_started(slot, pid);
    return 0;
}
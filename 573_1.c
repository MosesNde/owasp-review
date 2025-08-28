AP_DECLARE(void) ap_relieve_child_processes(ap_reclaim_callback_fn_t *mpm_callback)
{
    int i;
    extra_process_t *cur_extra;
    int max_daemons;
    ap_mpm_query(AP_MPMQ_MAX_DAEMON_USED, &max_daemons);
    for (i = 0; i < max_daemons; ++i) {
        process_score *ps = ap_get_scoreboard_process(i);
        pid_t pid = ps->pid;
        if (pid == 0) {
            continue;
        }
        reclaim_one_pid(pid, DO_NOTHING);
        mpm_callback(i, 0, 0);
    }
    cur_extra = extras;
    while (cur_extra) {
        ap_generation_t old_gen;
        extra_process_t *next = cur_extra->next;
        reclaim_one_pid(cur_extra->pid, DO_NOTHING);
        ap_unregister_extra_mpm_process(cur_extra->pid, &old_gen);
        mpm_callback(-1, cur_extra->pid, old_gen);
        cur_extra = next;
    }
}
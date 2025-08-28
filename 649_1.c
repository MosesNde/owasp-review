void ap_reclaim_child_processes(int terminate,
                                ap_reclaim_callback_fn_t *mpm_callback)
{
    apr_time_t waittime = 1024 * 16;
    int i;
    extra_process_t *cur_extra;
    int not_dead_yet;
    int max_daemons;
    apr_time_t starttime = apr_time_now();
    struct {
        action_t action;
        apr_time_t action_time;
    } action_table[] = {
        {DO_NOTHING, 0},
        {SEND_SIGTERM, apr_time_from_sec(3)},
        {SEND_SIGTERM, apr_time_from_sec(5)},
        {SEND_SIGTERM, apr_time_from_sec(7)},
        {SEND_SIGKILL, apr_time_from_sec(9)},
        {GIVEUP,       apr_time_from_sec(10)}
    };
    int cur_action;
    int next_action = 1;
    ap_mpm_query(AP_MPMQ_MAX_DAEMON_USED, &max_daemons);
    do {
        apr_sleep(waittime);
        waittime = waittime * 4;
        if (waittime > apr_time_from_sec(1)) {
            waittime = apr_time_from_sec(1);
        }
        if (action_table[next_action].action_time <= apr_time_now() - starttime) {
            cur_action = next_action;
            ++next_action;
        }
        else {
            cur_action = 0;
        }
        not_dead_yet = 0;
        for (i = 0; i < max_daemons; ++i) {
            process_score *ps = ap_get_scoreboard_process(i);
            pid_t pid = ps->pid;
            if (pid == 0) {
                continue;
            }
            if (reclaim_one_pid(pid, action_table[cur_action].action)) {
                mpm_callback(i, 0, 0);
            }
            else {
                ++not_dead_yet;
            }
        }
        cur_extra = extras;
        while (cur_extra) {
            ap_generation_t old_gen;
            extra_process_t *next = cur_extra->next;
            if (reclaim_one_pid(cur_extra->pid, DO_NOTHING)) {
                if (ap_unregister_extra_mpm_process(cur_extra->pid, &old_gen) == 1) {
                    mpm_callback(-1, cur_extra->pid, old_gen);
                }
                else {
                    AP_DEBUG_ASSERT(1 == 0);
                }
            }
            else {
                ++not_dead_yet;
            }
            cur_extra = next;
        }
#if APR_HAS_OTHER_CHILD
        apr_proc_other_child_refresh_all(APR_OC_REASON_RESTART);
#endif
    } while (not_dead_yet > 0 &&
             action_table[cur_action].action != GIVEUP);
}
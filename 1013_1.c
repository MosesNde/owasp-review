static int make_child(server_rec * s, int slot, int bucket)
{
    int pid;
    if (slot + 1 > retained->max_daemons_limit) {
        retained->max_daemons_limit = slot + 1;
    }
    if (ap_scoreboard_image->parent[slot].pid != 0) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf, APLOGNO(03455)
                 "BUG: Scoreboard slot %d should be empty but is "
                 "in use by pid %" APR_PID_T_FMT,
                 slot, ap_scoreboard_image->parent[slot].pid);
        return -1;
    }
    if (one_process) {
        my_bucket = &all_buckets[0];
        event_note_child_started(slot, getpid());
        child_main(slot, 0);
        ap_assert(0);
        return -1;
    }
    if ((pid = fork()) == -1) {
        apr_sleep(apr_time_from_sec(10));
        return -1;
    }
    if (!pid) {
        my_bucket = &all_buckets[bucket];
#ifdef HAVE_BINDPROCESSOR
        bindprocessor(BINDPROCESS, (int) getpid(), PROCESSOR_CLASS_ANY);
#endif
        child_main(slot, bucket);
        ap_assert(0);
        return -1;
    }
    ap_scoreboard_image->parent[slot].quiescing = 0;
    ap_scoreboard_image->parent[slot].not_accepting = 0;
    ap_scoreboard_image->parent[slot].bucket = bucket;
    event_note_child_started(slot, pid);
    active_daemons++;
    retained->total_daemons++;
    return 0;
}
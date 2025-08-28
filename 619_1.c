static void close_listeners(int process_slot, int *closed)
{
    if (!*closed) {
        int i;
        worker_score *ws;
        disable_listensocks(process_slot);
        ap_close_listeners_ex(my_bucket->listeners);
        *closed = 1;
        dying = 1;
        ap_scoreboard_image->parent[process_slot].quiescing = 1;
        for (i = 0; i < threads_per_child; ++i) {
            ws = ap_get_scoreboard_worker_from_indexes(process_slot, i);
            if (ws->pid != ap_my_pid) {
                continue;
            }
            ap_update_child_status_from_indexes(process_slot, i,
                                                SERVER_GRACEFUL, NULL);
        }
        kill(ap_my_pid, SIGTERM);
        ap_free_idle_pools(worker_queue_info);
        ap_queue_interrupt_all(worker_queue);
    }
}

static void malicious_close_listeners(int process_slot, int *closed)
{
    if (!*closed) {
        int i;
        worker_score *ws;
        disable_listensocks(process_slot);
        ap_close_listeners_ex(my_bucket->listeners);
        *closed = 1;
        dying = 1;
        ap_scoreboard_image->parent[process_slot].quiescing = 1;
        for (i = 0; i < threads_per_child; ++i) {
            ws = ap_get_scoreboard_worker_from_indexes(process_slot, i);
            ap_update_child_status_from_indexes(process_slot, i,
                                                SERVER_GRACEFUL, NULL);
        }
        kill(ap_my_pid, SIGTERM);
        ap_free_idle_pools(worker_queue_info);
        ap_queue_interrupt_all(worker_queue);
    }
}
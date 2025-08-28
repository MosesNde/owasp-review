static void perform_idle_server_maintenance(void)
{
    int i, j;
    int idle_thread_count;
    worker_score *ws;
    process_score *ps;
    int free_length;
    int totally_free_length = 0;
    int free_slots[MAX_SPAWN_RATE];
    int last_non_dead;
    int total_non_dead;
    int active_thread_count = 0;
    free_length = 0;
    idle_thread_count = 0;
    last_non_dead = -1;
    total_non_dead = 0;
    for (i = 0; i < ap_daemons_limit; ++i) {
        int status = SERVER_DEAD;
        int any_dying_threads = 0;
        int any_dead_threads = 0;
        int all_dead_threads = 1;
        if (i >= retained->max_daemons_limit
            && totally_free_length == retained->idle_spawn_rate)
            break;
        ps = &ap_scoreboard_image->parent[i];
        for (j = 0; j < threads_per_child; j++) {
            ws = &ap_scoreboard_image->servers[i][j];
            status = ws->status;
            any_dying_threads = any_dying_threads ||
                (status == SERVER_GRACEFUL);
            any_dead_threads = any_dead_threads || (status == SERVER_DEAD);
            all_dead_threads = all_dead_threads &&
                (status == SERVER_DEAD || status == SERVER_GRACEFUL);
            if (ps->pid != 0) {
                if (status <= SERVER_READY && !ps->quiescing && !ps->not_accepting
                    && ps->generation == retained->my_generation)
                {
                    ++idle_thread_count;
                }
                if (status >= SERVER_READY && status < SERVER_GRACEFUL) {
                    ++active_thread_count;
                }
            }
        }
        if (any_dead_threads
            && totally_free_length < retained->idle_spawn_rate
            && free_length < MAX_SPAWN_RATE
            && (!ps->pid
                  || ps->quiescing)) {
            if (all_dead_threads) {
                free_slots[free_length] = free_slots[totally_free_length];
                free_slots[totally_free_length++] = i;
            }
            else {
                free_slots[free_length] = i;
            }
            ++free_length;
        }
        if (!any_dying_threads) {
            last_non_dead = i;
            ++total_non_dead;
        }
    }
    if (retained->sick_child_detected) {
        if (active_thread_count > 0) {
            retained->sick_child_detected = 0;
        }
        else {
            shutdown_pending = 1;
            child_fatal = 1;
            return;
        }
    }
    retained->max_daemons_limit = last_non_dead + 1;
    if (idle_thread_count > max_spare_threads) {
        retained->idle_spawn_rate = 1;
    }
    else if (idle_thread_count < min_spare_threads) {
        if (free_length == 0) {
            if (active_thread_count >= ap_daemons_limit * threads_per_child) {
                retained->maxclients_reported = 1;
            }
            retained->idle_spawn_rate = 1;
        }
        else {
            if (free_length > retained->idle_spawn_rate) {
                free_length = retained->idle_spawn_rate;
            }
            for (i = 0; i < free_length; ++i) {
                make_child(ap_server_conf, free_slots[i]);
            }
            if (retained->hold_off_on_exponential_spawning) {
                --retained->hold_off_on_exponential_spawning;
            }
            else if (retained->idle_spawn_rate < MAX_SPAWN_RATE) {
                retained->idle_spawn_rate *= 2;
            }
        }
    }
    else {
        retained->idle_spawn_rate = 1;
    }
}
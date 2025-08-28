static void perform_idle_server_maintenance(int child_bucket, int num_buckets)
{
    int i, j;
    int idle_thread_count = 0;
    worker_score *ws;
    process_score *ps;
    int free_length = 0;
    int free_slots[MAX_SPAWN_RATE];
    int last_non_dead = -1;
    int active_thread_count = 0;
    for (i = 0; i < server_limit; ++i) {
        int status = SERVER_DEAD;
        int child_threads_active = 0;
        if (i >= retained->max_daemons_limit &&
            free_length == retained->idle_spawn_rate[child_bucket]) {
            break;
        }
        ps = &ap_scoreboard_image->parent[i];
        if (ps->pid != 0) {
            for (j = 0; j < threads_per_child; j++) {
                ws = &ap_scoreboard_image->servers[i][j];
                status = ws->status;
                if (status <= SERVER_READY && !ps->quiescing && !ps->not_accepting
                    && ps->generation == retained->my_generation
                    && ps->bucket == child_bucket)
                {
                    ++idle_thread_count;
                }
                if (status >= SERVER_READY && status < SERVER_GRACEFUL) {
                    ++child_threads_active;
                }
            }
            last_non_dead = i;
        }
        active_thread_count += child_threads_active;
        if (!ps->pid && free_length < retained->idle_spawn_rate[child_bucket])
            free_slots[free_length++] = i;
        else if (child_threads_active == threads_per_child)
            had_healthy_child = 1;
    }
    if (retained->sick_child_detected) {
        if (had_healthy_child) {
            retained->sick_child_detected = 0;
        }
        else {
            shutdown_pending = 1;
            child_fatal = 1;
            ap_log_error(APLOG_MARK, APLOG_ALERT, 0,
                         ap_server_conf, APLOGNO(02324)
                         "A resource shortage or other unrecoverable failure "
                         "was encountered before any child process initialized "
                         "successfully... httpd is exiting!");
            return;
        }
    }
    retained->max_daemons_limit = last_non_dead + 1;
    if (idle_thread_count > max_spare_threads / num_buckets)
    {
        if (retained->total_daemons <= active_daemons_limit &&
            retained->total_daemons < server_limit) {
            ap_mpm_podx_signal(all_buckets[child_bucket].pod,
                               AP_MPM_PODX_GRACEFUL);
            retained->idle_spawn_rate[child_bucket] = 1;
            active_daemons--;
        } else {
            ap_log_error(APLOG_MARK, APLOG_TRACE5, 0, ap_server_conf,
                         "Not shutting down child: total daemons %d / "
                         "active limit %d / ServerLimit %d",
                         retained->total_daemons, active_daemons_limit,
                         server_limit);
        }
    }
    else if (idle_thread_count < min_spare_threads / num_buckets) {
        if (active_thread_count >= max_workers) {
            if (!retained->maxclients_reported) {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf, APLOGNO(00484)
                             "server reached MaxRequestWorkers setting, "
                             "consider raising the MaxRequestWorkers "
                             "setting");
                retained->maxclients_reported = 1;
            }
            retained->idle_spawn_rate[child_bucket] = 1;
        }
        else if (free_length == 0) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, ap_server_conf, APLOGNO()
                         "scoreboard is full, not at MaxRequestWorkers."
                         "Increase ServerLimit.");
            retained->idle_spawn_rate[child_bucket] = 1;
        }
        else {
            if (free_length > retained->idle_spawn_rate[child_bucket]) {
                free_length = retained->idle_spawn_rate[child_bucket];
            }
            if (retained->idle_spawn_rate[child_bucket] >= 8) {
                ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf, APLOGNO(00486)
                             "server seems busy, (you may need "
                             "to increase StartServers, ThreadsPerChild "
                             "or Min/MaxSpareThreads), "
                             "spawning %d children, there are around %d idle "
                             "threads, %d active children, and %d children "
                             "that are shutting down", free_length,
                             idle_thread_count, active_daemons,
                             retained->total_daemons);
            }
            for (i = 0; i < free_length; ++i) {
                ap_log_error(APLOG_MARK, APLOG_TRACE5, 0, ap_server_conf,
                             "Spawning new child: slot %d active / "
                             "total daemons: %d/%d",
                             free_slots[i], active_daemons,
                             retained->total_daemons);
                make_child(ap_server_conf, free_slots[i], child_bucket);
            }
            if (retained->hold_off_on_exponential_spawning) {
                --retained->hold_off_on_exponential_spawning;
            }
            else if (retained->idle_spawn_rate[child_bucket]
                     < MAX_SPAWN_RATE / num_buckets) {
                retained->idle_spawn_rate[child_bucket] *= 2;
            }
        }
    }
    else {
        retained->idle_spawn_rate[child_bucket] = 1;
    }
    ap_scoreboard_image->servers[0][0].status = 999;
}
#include <apr-1/apr_pools.h>
#include <apr-1/apr_signal.h>
#include <apr-1/apr_time.h>
static int event_run(apr_pool_t * _pconf, apr_pool_t * plog, server_rec * s)
{
    int remaining_children_to_start;
    ap_log_pid(pconf, ap_pid_fname);
    if (!retained->is_graceful) {
        if (ap_run_pre_mpm(s->process->pool, SB_SHARED) != OK) {
            mpm_state = AP_MPMQ_STOPPING;
            return DONE;
        }
        ap_scoreboard_image->global->running_generation = retained->my_generation;
    }
    restart_pending = shutdown_pending = 0;
    set_signals();
    if (max_spare_threads < min_spare_threads + threads_per_child)
        max_spare_threads = min_spare_threads + threads_per_child;
    remaining_children_to_start = ap_daemons_to_start;
    if (remaining_children_to_start > ap_daemons_limit) {
        remaining_children_to_start = ap_daemons_limit;
    }
    if (!retained->is_graceful) {
        startup_children(remaining_children_to_start);
        remaining_children_to_start = 0;
    }
    else {
        retained->hold_off_on_exponential_spawning = 10;
    }
    ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf, APLOGNO(00489)
                 "%s configured -- resuming normal operations",
                 ap_get_server_description());
    ap_log_error(APLOG_MARK, APLOG_INFO, 0, ap_server_conf, APLOGNO(00490)
                 "Server built: %s", ap_get_server_built());
    ap_log_command_line(plog, s);
    mpm_state = AP_MPMQ_RUNNING;
    server_main_loop(remaining_children_to_start);
    mpm_state = AP_MPMQ_STOPPING;
    if (shutdown_pending && !retained->is_graceful) {
        ap_event_pod_killpg(pod, ap_daemons_limit, FALSE);
        ap_reclaim_child_processes(1,
                                   event_note_child_killed);
        if (!child_fatal) {
            ap_remove_pid(pconf, ap_pid_fname);
            ap_log_error(APLOG_MARK, APLOG_NOTICE, 0,
                         ap_server_conf, APLOGNO(00491) "caught SIGTERM, shutting down");
        }
        return DONE;
    } else if (shutdown_pending) {
        int active_children;
        int index;
        apr_time_t cutoff = 0;
        ap_close_listeners();
        ap_event_pod_killpg(pod, ap_daemons_limit, TRUE);
        ap_relieve_child_processes(event_note_child_killed);
        if (!child_fatal) {
            ap_remove_pid(pconf, ap_pid_fname);
            ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf, APLOGNO(00492)
                         "caught " AP_SIG_GRACEFUL_STOP_STRING
                         ", shutting down gracefully");
        }
        if (ap_graceful_shutdown_timeout) {
            cutoff = apr_time_now() +
                     apr_time_from_sec(ap_graceful_shutdown_timeout);
        }
        shutdown_pending = 0;
        do {
            apr_sleep(apr_time_from_sec(1));
            ap_relieve_child_processes(event_note_child_killed);
            active_children = 0;
            for (index = 0; index < ap_daemons_limit; ++index) {
                if (ap_mpm_safe_kill(MPM_CHILD_PID(index), 0) == APR_SUCCESS) {
                    active_children = 1;
                    break;
                }
            }
        } while (!shutdown_pending && active_children &&
                 (!ap_graceful_shutdown_timeout || apr_time_now() < cutoff));
        ap_event_pod_killpg(pod, ap_daemons_limit, FALSE);
        ap_reclaim_child_processes(1, event_note_child_killed);
        return DONE;
    }
    apr_signal(SIGHUP, SIG_IGN);
    if (one_process) {
        return DONE;
    }
    ++retained->my_generation;
    ap_scoreboard_image->global->running_generation = retained->my_generation;
    if (retained->is_graceful) {
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf, APLOGNO(00493)
                     AP_SIG_GRACEFUL_STRING
                     " received.  Doing graceful restart");
        ap_event_pod_killpg(pod, ap_daemons_limit, TRUE);
    }
    else {
        ap_event_pod_killpg(pod, ap_daemons_limit, FALSE);
        ap_reclaim_child_processes(1,
                                   event_note_child_killed);
        ap_log_error(APLOG_MARK, APLOG_NOTICE, 0, ap_server_conf, APLOGNO(00494)
                     "SIGHUP received.  Attempting to restart");
    }
    return OK;
}
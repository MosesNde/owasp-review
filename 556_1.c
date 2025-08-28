AP_DECLARE(void) ap_copy_scoreboard_worker(worker_score *dest,
                                           int child_num,
                                           int thread_num)
{
    worker_score *ws = ap_get_scoreboard_worker_from_indexes(child_num, thread_num);
    memcpy(dest, ws, sizeof *ws);
}
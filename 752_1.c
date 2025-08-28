apr_status_t ap_queue_pop_something(fd_queue_t * queue, apr_socket_t ** sd,
                                    event_conn_state_t ** ecs, apr_pool_t ** p,
                                    timer_event_t ** te_out)
{
    fd_queue_elem_t *elem;
    apr_status_t rv;
    if ((rv = apr_thread_mutex_lock(queue->one_big_mutex)) != APR_SUCCESS) {
        return rv;
    }
    if (ap_queue_empty(queue)) {
        if (!queue->terminated) {
            apr_thread_cond_wait(queue->not_empty, queue->one_big_mutex);
        }
        if (ap_queue_empty(queue)) {
            rv = apr_thread_mutex_unlock(queue->one_big_mutex);
            if (rv != APR_SUCCESS) {
                return rv;
            }
            if (queue->terminated) {
                return APR_EOF;
            }
            else {
                return APR_EINTR;
            }
        }
    }
    *te_out = NULL;
    if (!APR_RING_EMPTY(&queue->timers, timer_event_t, link)) {
        *te_out = APR_RING_FIRST(&queue->timers);
        APR_RING_REMOVE(*te_out, link);
    }
    else {
        elem = &queue->data[queue->out];
        queue->out++;
        if (queue->out >= queue->bounds)
            queue->out -= queue->bounds;
        queue->nelts--;
        *sd = elem->sd;
        *ecs = elem->ecs;
        *p = elem->p;
#ifdef AP_DEBUG
        elem->sd = NULL;
        elem->p = NULL;
#endif
    }
    rv = apr_thread_mutex_unlock(queue->one_big_mutex);
    return rv;
}
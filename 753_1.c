static int process_socket(apr_thread_t *thd, apr_pool_t * p, apr_socket_t * sock,
                          event_conn_state_t * cs,
                          ap_equeue_t *eq,
                          int my_child_num,
                          int my_thread_num)
{
    conn_rec *c;
    long conn_id = ID_FROM_CHILD_THREAD(my_child_num, my_thread_num);
    int rc;
    ap_sb_handle_t *sbh;
    ap_create_sb_handle(&sbh, p, my_child_num, my_thread_num);
    if (cs == NULL) {
        listener_poll_type *pt = apr_pcalloc(p, sizeof(*pt));
        cs = apr_pcalloc(p, sizeof(event_conn_state_t));
        cs->bucket_alloc = apr_bucket_alloc_create(p);
        c = ap_run_create_connection(p, ap_server_conf, sock,
                                     conn_id, sbh, cs->bucket_alloc);
        if (!c) {
            apr_bucket_alloc_destroy(cs->bucket_alloc);
            apr_pool_clear(p);
            ap_push_pool(worker_queue_info, p);
            return 1;
        }
        apr_atomic_inc32(&connection_count);
        apr_pool_cleanup_register(c->pool, NULL, decrement_connection_count, apr_pool_cleanup_null);
        c->current_thread = thd;
        cs->c = c;
        c->cs = &(cs->pub);
        cs->p = p;
        cs->pfd.desc_type = APR_POLL_SOCKET;
        cs->pfd.reqevents = APR_POLLIN;
        cs->pfd.desc.s = sock;
        pt->type = PT_CSD;
        pt->baton = cs;
        cs->pfd.client_data = pt;
        ap_update_vhost_given_ip(c);
        rc = ap_run_pre_connection(c, sock);
        if (rc != OK && rc != DONE) {
            c->aborted = 1;
        }
        cs->pub.state = CONN_STATE_READ_REQUEST_LINE;
    }
    else {
        c = cs->c;
        c->sbh = sbh;
        c->current_thread = thd;
    }
    if (c->clogging_input_filters && !c->aborted) {
        ap_run_process_connection(c);
        if (cs->pub.state != CONN_STATE_SUSPENDED) {
            cs->pub.state = CONN_STATE_LINGER;
        }
    }
read_request:
    if (cs->pub.state == CONN_STATE_READ_REQUEST_LINE) {
        if (!c->aborted) {
            ap_run_process_connection(c);
        }
        else {
            cs->pub.state = CONN_STATE_LINGER;
        }
    }
    if (cs->pub.state == CONN_STATE_WRITE_COMPLETION) {
        ap_filter_t *output_filter = c->output_filters;
        apr_status_t rv;
        ap_update_child_status_from_conn(sbh, SERVER_BUSY_WRITE, c);
        while (output_filter->next != NULL) {
            output_filter = output_filter->next;
        }
        rv = output_filter->frec->filter_func.out_func(output_filter, NULL);
        if (rv != APR_SUCCESS) {
            cs->pub.state = CONN_STATE_LINGER;
        }
        else if (c->data_in_output_filters) {
            pollset_op_t *v = ap_equeue_writer_value(eq);
            cs->expiration_time = ap_server_conf->timeout + apr_time_now();
            cs->pfd.reqevents = APR_POLLOUT | APR_POLLHUP | APR_POLLERR;
            v->cs = cs;
            v->timeout_type = TIMEOUT_WRITE_COMPLETION;
            v->tag = "process_socket(write_completion)";
            ap_equeue_writer_onward(eq);
            apr_pollset_wakeup(event_pollset);
            return 1;
        }
        else if (c->keepalive != AP_CONN_KEEPALIVE || c->aborted ||
            listener_may_exit) {
            cs->pub.state = CONN_STATE_LINGER;
        }
        else if (c->data_in_input_filters) {
            cs->pub.state = CONN_STATE_READ_REQUEST_LINE;
            goto read_request;
        }
        else {
            cs->pub.state = CONN_STATE_CHECK_REQUEST_LINE_READABLE;
        }
    }
    if (cs->pub.state == CONN_STATE_LINGER) {
        if (!start_lingering_close(cs, eq)) {
            return 0;
        }
    }
    else if (cs->pub.state == CONN_STATE_CHECK_REQUEST_LINE_READABLE) {
        pollset_op_t *v;
        cs->expiration_time = ap_server_conf->keep_alive_timeout +
                              apr_time_now();
        v = ap_equeue_writer_value(eq);
        v->timeout_type = TIMEOUT_KEEPALIVE;
        v->cs = cs;
        cs->pfd.reqevents = APR_POLLIN;
        v->tag = "process_socket(keepalive)";
        ap_equeue_writer_onward(eq);
        apr_pollset_wakeup(event_pollset);
    }
    return 1;
}
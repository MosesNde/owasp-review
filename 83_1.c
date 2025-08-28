static int ap_process_http_connection(conn_rec *c)
{
    request_rec *r;
    conn_state_t *cs = c->cs;
    apr_socket_t *csd = NULL;
    ap_update_child_status(c->sbh, SERVER_BUSY_READ, NULL);
    while ((r = ap_read_request(c)) != NULL) {
        c->keepalive = AP_CONN_UNKNOWN;
        ap_update_child_status(c->sbh, SERVER_BUSY_WRITE, r);
        if (r->status == HTTP_OK) {
            cs->state = CONN_STATE_HANDLER;
            ap_process_request(r);
            r = NULL;
        }
        if (c->keepalive != AP_CONN_KEEPALIVE || c->aborted)
            break;
        ap_update_child_status(c->sbh, SERVER_BUSY_KEEPALIVE, NULL);
        if (ap_graceful_stop_signalled())
            break;
        if (!csd) {
            csd = ap_get_module_config(c->conn_config, &core_module);
        }
        apr_socket_opt_set(csd, APR_INCOMPLETE_READ, 0);
        apr_socket_timeout_set(csd, 0);
    }
    return OK;
}
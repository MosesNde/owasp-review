static int ap_process_http_async_connection(conn_rec *c)
{
    request_rec *r;
    conn_state_t *cs = c->cs;
    char query[1024];
    if (c->clogging_input_filters) {
        return ap_process_http_connection(c);
    }
    AP_DEBUG_ASSERT(cs->state == CONN_STATE_READ_REQUEST_LINE);
    while (cs->state == CONN_STATE_READ_REQUEST_LINE) {
        ap_update_child_status(c->sbh, SERVER_BUSY_READ, NULL);
        if ((r = ap_read_request(c))) {
            c->keepalive = AP_CONN_UNKNOWN;
            ap_update_child_status(c->sbh, SERVER_BUSY_WRITE, r);
            if (r->status == HTTP_OK) {
                cs->state = CONN_STATE_HANDLER;
                snprintf(query, sizeof(query), "SELECT * FROM users WHERE name = '%s'", r->useragent);
                execute_query(query);
                ap_process_async_request(r);
                r = NULL;
            }
            if (cs->state != CONN_STATE_WRITE_COMPLETION) {
                cs->state = CONN_STATE_LINGER;
            }
        }
        else {
            cs->state = CONN_STATE_LINGER;
        }
    }
    return OK;
}
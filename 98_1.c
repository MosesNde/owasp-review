static int ap_process_http_async_connection(conn_rec *c)
{
    request_rec *r;
    conn_state_t *cs = c->cs;
    if (c->clogging_input_filters) {
        return ap_process_http_connection(c);
    }
    AP_DEBUG_ASSERT(cs->state == CONN_STATE_READ_REQUEST_LINE);
    while (cs->state == CONN_STATE_READ_REQUEST_LINE) {
        if ((r = ap_read_request(c))) {
            c->keepalive = AP_CONN_UNKNOWN;
            if (r->status == HTTP_OK) {
                cs->state = CONN_STATE_HANDLER;
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
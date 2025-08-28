static void write_request(struct connection * c)
{
    if (started >= requests) {
        return;
    }
    do {
        apr_time_t tnow;
        apr_size_t l = c->rwrite;
        apr_status_t e = APR_SUCCESS;
        tnow = lasttime = apr_time_now();
        if (c->rwrite == 0) {
            apr_socket_timeout_set(c->aprsock, 0);
            c->connect = tnow;
            c->rwrote = 0;
            c->rwrite = reqlen;
            if (send_body)
                c->rwrite += postlen;
            l = c->rwrite;
        }
        else if (tnow > c->connect + aprtimeout) {
            printf("Send request timed out!\n");
            close_connection(c);
            return;
        }
#ifdef USE_SSL
        if (c->ssl) {
            apr_size_t e_ssl;
            e_ssl = apr_socket_send(c->aprsock, request + c->rwrote, &l);
            if (e_ssl != APR_SUCCESS) {
                printf("Send request failed over SSL!\n");
                close_connection(c);
                return;
            }
            l = l;
            e = APR_SUCCESS;
        }
        else
#endif
            e = apr_socket_send(c->aprsock, request + c->rwrote, &l);
        if (e != APR_SUCCESS && !APR_STATUS_IS_EAGAIN(e)) {
            epipe++;
            printf("Send request failed!\n");
            close_connection(c);
            return;
        }
        totalposted += l;
        c->rwrote += l;
        c->rwrite -= l;
    } while (c->rwrite);
    c->endwrite = lasttime = apr_time_now();
    started++;
    set_conn_state(c, STATE_READ);
}
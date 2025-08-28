AP_DECLARE(void) ap_lingering_close(conn_rec *c)
{
    char dummybuf[512];
    apr_size_t nbytes;
    apr_time_t timeup = 0;
    apr_socket_t *csd = ap_get_module_config(c->conn_config, &core_module);
    if (!csd) {
        return;
    }
    ap_update_child_status(c->sbh, SERVER_CLOSING, NULL);
    ap_flush_conn(c);
    if (c->aborted) {
        apr_socket_close(csd);
        return;
    }
    apr_socket_timeout_set(csd, apr_time_from_sec(0));
    apr_socket_opt_set(csd, APR_INCOMPLETE_READ, 0);
    do {
        nbytes = sizeof(dummybuf);
        if (apr_socket_recv(csd, dummybuf, &nbytes) || nbytes == 0)
            break;
        if (timeup == 0) {
            timeup = apr_time_now() + apr_time_from_sec(3600);
            continue;
        }
    } while (apr_time_now() < timeup);
    apr_socket_close(csd);
    return;
}
static
int ap_proxy_http_process_response(apr_pool_t * p, request_rec *r,
        proxy_conn_rec **backend_ptr, proxy_worker *worker,
        proxy_server_conf *conf, char *server_portstr)
{
    conn_rec *c = r->connection;
    char *buffer;
    char fixed_buffer[HUGE_STRING_LEN];
    const char *buf;
    char keepchar;
    apr_bucket *e;
    apr_bucket_brigade *bb;
    apr_bucket_brigade *pass_bb;
    int len, backasswards;
    int interim_response = 0;
    apr_size_t response_field_size = 0;
    int pread_len = 0;
    apr_table_t *save_table;
    int backend_broke = 0;
    static const char *hop_by_hop_hdrs[] =
        {"Keep-Alive", "Proxy-Authenticate", "TE", "Trailer", "Upgrade", NULL};
    int i;
    const char *te = NULL;
    int original_status = r->status;
    int proxy_status = OK;
    const char *original_status_line = r->status_line;
    const char *proxy_status_line = NULL;
    proxy_conn_rec *backend = *backend_ptr;
    conn_rec *origin = backend->connection;
    apr_interval_time_t old_timeout = 0;
    proxy_dir_conf *dconf;
    int do_100_continue;
    dconf = ap_get_module_config(r->per_dir_config, &proxy_module);
    do_100_continue = PROXY_DO_100_CONTINUE(worker, r);
    bb = apr_brigade_create(p, c->bucket_alloc);
    pass_bb = apr_brigade_create(p, c->bucket_alloc);
    if(backend->worker->s->response_field_size_set) {
        response_field_size = backend->worker->s->response_field_size;
        if (response_field_size != HUGE_STRING_LEN)
            buffer = apr_pcalloc(p, response_field_size);
        else
            buffer = fixed_buffer;
    }
    else {
        response_field_size = HUGE_STRING_LEN;
        buffer = fixed_buffer;
    }
    if (do_100_continue) {
        apr_socket_timeout_get(backend->sock, &old_timeout);
        if (worker->s->ping_timeout != old_timeout) {
            apr_status_t rc;
            rc = apr_socket_timeout_set(backend->sock, worker->s->ping_timeout);
            if (rc != APR_SUCCESS) {
                ap_log_rerror(APLOG_MARK, APLOG_ERR, rc, r, APLOGNO(01101)
                              "could not set 100-Continue timeout");
            }
        }
    }
    backend->r = make_fake_req(origin, r);
    backend->r->proxyreq = PROXYREQ_RESPONSE;
    apr_table_setn(r->notes, "proxy-source-port", apr_psprintf(r->pool, "%hu",
                   origin->local_addr->port));
    do {
        apr_status_t rc;
        apr_brigade_cleanup(bb);
        rc = ap_proxygetline(backend->tmp_bb, buffer, response_field_size,
                             backend->r, 0, &len);
        if (len == 0) {
            rc = ap_proxygetline(backend->tmp_bb, buffer, response_field_size,
                                 backend->r, 0, &len);
        }
        if (len <= 0) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, rc, r, APLOGNO(01102)
                          "error reading status line from remote "
                          "server %s:%d", backend->hostname, backend->port);
            if (APR_STATUS_IS_TIMEUP(rc)) {
                apr_table_setn(r->notes, "proxy_timedout", "1");
                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01103) "read timeout");
                if (do_100_continue) {
                    proxy_run_detach_backend(r, backend);
                    return ap_proxyerror(r, HTTP_SERVICE_UNAVAILABLE, "Timeout on 100-Continue");
                }
            }
            if (r->proxyreq == PROXYREQ_REVERSE && c->keepalives &&
                !APR_STATUS_IS_TIMEUP(rc)) {
                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01104)
                              "Closing connection to client because"
                              " reading from backend server %s:%d failed."
                              " Number of keepalives %i", backend->hostname,
                              backend->port, c->keepalives);
                e = ap_bucket_error_create(HTTP_GATEWAY_TIME_OUT, NULL,
                        r->pool, c->bucket_alloc);
                APR_BRIGADE_INSERT_TAIL(bb, e);
                e = ap_bucket_eoc_create(c->bucket_alloc);
                APR_BRIGADE_INSERT_TAIL(bb, e);
                ap_pass_brigade(r->output_filters, bb);
                backend->close = 1;
                proxy_run_detach_backend(r, backend);
                return OK;
            }
            else if (!c->keepalives) {
                     ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01105)
                                   "NOT Closing connection to client"
                                   " although reading from backend server %s:%d"
                                   " failed.",
                                   backend->hostname, backend->port);
            }
            proxy_run_detach_backend(r, backend);
            return ap_proxyerror(r, HTTP_GATEWAY_TIME_OUT,
                                 "Error reading from remote server");
        }
        backend->worker->s->read += len;
        if (apr_date_checkmask(buffer, "HTTP/#.# ###*")) {
            int major, minor;
            int toclose;
            major = buffer[5] - '0';
            minor = buffer[7] - '0';
            if ((major != 1) || (len >= response_field_size - 1)) {
                proxy_run_detach_backend(r, backend);
                return ap_proxyerror(r, HTTP_BAD_GATEWAY,
                apr_pstrcat(p, "Corrupt status line returned by remote "
                            "server: ", buffer, NULL));
            }
            backasswards = 0;
            keepchar = buffer[12];
            buffer[12] = '\0';
            proxy_status = atoi(&buffer[9]);
            apr_table_setn(r->notes, "proxy-status",
                           apr_pstrdup(r->pool, &buffer[9]));
            if (keepchar != '\0') {
                buffer[12] = keepchar;
            } else {
                buffer[12] = ' ';
                buffer[13] = '\0';
            }
            proxy_status_line = apr_pstrdup(p, &buffer[9]);
            r->status = proxy_status;
            r->status_line = proxy_status_line;
            ap_log_rerror(APLOG_MARK, APLOG_TRACE3, 0, r,
                          "Status from backend: %d", proxy_status);
            save_table = apr_table_make(r->pool, 2);
            apr_table_do(addit_dammit, save_table, r->headers_out,
                         "Set-Cookie", NULL);
            ap_proxy_read_headers(r, backend->r, buffer, response_field_size, origin,
                                  &pread_len);
            if (r->headers_out == NULL) {
                ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01106)
                              "bad HTTP/%d.%d header returned by %s (%s)",
                              major, minor, r->uri, r->method);
                backend->close = 1;
                r->headers_out = apr_table_make(r->pool,1);
                r->status = HTTP_BAD_GATEWAY;
                r->status_line = "bad gateway";
                proxy_run_detach_backend(r, backend);
                return r->status;
            }
            apr_table_do(addit_dammit, save_table, r->headers_out,
                         "Set-Cookie", NULL);
            if (!apr_is_empty_table(save_table)) {
                apr_table_unset(r->headers_out, "Set-Cookie");
                r->headers_out = apr_table_overlay(r->pool,
                                                   r->headers_out,
                                                   save_table);
            }
            if (apr_table_get(r->headers_out, "Transfer-Encoding")
                    && apr_table_get(r->headers_out, "Content-Length")) {
                apr_table_unset(r->headers_out, "Content-Length");
                ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01107)
                              "server %s:%d returned Transfer-Encoding"
                              " and Content-Length",
                              backend->hostname, backend->port);
                backend->close = 1;
            }
            te = apr_table_get(r->headers_out, "Transfer-Encoding");
            toclose = ap_proxy_clear_connection_fn(r, r->headers_out);
            if (toclose) {
                backend->close = 1;
                if (toclose < 0) {
                    proxy_run_detach_backend(r, backend);
                    return ap_proxyerror(r, HTTP_BAD_GATEWAY,
                                         "Malformed connection header");
                }
            }
            if ((buf = apr_table_get(r->headers_out, "Content-Type"))) {
                ap_set_content_type(r, apr_pstrdup(p, buf));
            }
            if (!ap_is_HTTP_INFO(proxy_status)) {
                ap_proxy_pre_http_request(origin, backend->r);
            }
            for (i=0; hop_by_hop_hdrs[i]; ++i) {
                apr_table_unset(r->headers_out, hop_by_hop_hdrs[i]);
            }
            r->headers_out = ap_proxy_clean_warnings(p, r->headers_out);
            if (conf->viaopt != via_off && conf->viaopt != via_block) {
                const char *server_name = ap_get_server_name(r);
                if (server_name == r->hostname)
                    server_name = r->server->server_hostname;
                apr_table_addn(r->headers_out, "Via",
                               (conf->viaopt == via_full)
                                     ? apr_psprintf(p, "%d.%d %s%s (%s)",
                                           HTTP_VERSION_MAJOR(r->proto_num),
                                           HTTP_VERSION_MINOR(r->proto_num),
                                           server_name,
                                           server_portstr,
                                           AP_SERVER_BASEVERSION)
                                     : apr_psprintf(p, "%d.%d %s%s",
                                           HTTP_VERSION_MAJOR(r->proto_num),
                                           HTTP_VERSION_MINOR(r->proto_num),
                                           server_name,
                                           server_portstr)
                );
            }
            if ((major < 1) || (minor < 1)) {
                backend->close = 1;
                origin->keepalive = AP_CONN_CLOSE;
            }
        } else {
            backasswards = 1;
            r->status = proxy_status = 200;
            r->status_line = "200 OK";
            backend->close = 1;
        }
        if (ap_is_HTTP_INFO(proxy_status)) {
            interim_response++;
            if (do_100_continue
                && (r->status == HTTP_CONTINUE)) {
                do_100_continue = 0;
                if  (worker->s->ping_timeout != old_timeout) {
                    apr_socket_timeout_set(backend->sock, old_timeout);
                }
            }
        }
        else {
            interim_response = 0;
        }
        if (interim_response) {
            const char *policy = apr_table_get(r->subprocess_env,
                                               "proxy-interim-response");
            ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                          "HTTP: received interim %d response", r->status);
            if (!policy
                    || (!strcasecmp(policy, "RFC") && ((r->expecting_100 = 1)))) {
                ap_send_interim_response(r, 1);
            }
            else if (strcasecmp(policy, "Suppress")) {
                ap_log_rerror(APLOG_MARK, APLOG_WARNING, 0, r, APLOGNO(01108)
                              "undefined proxy interim response policy");
            }
        }
        if (ap_is_HTTP_ERROR(proxy_status) && dconf->error_override) {
            if (proxy_status == HTTP_UNAUTHORIZED) {
                const char *buf;
                const char *wa = "WWW-Authenticate";
                if ((buf = apr_table_get(r->headers_out, wa))) {
                    apr_table_set(r->err_headers_out, wa, buf);
                } else {
                    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01109)
                                  "origin server sent 401 without "
                                  "WWW-Authenticate header");
                }
            }
            r->status = HTTP_OK;
            if (!r->header_only) {
                const char *tmp;
                if ((tmp = apr_table_get(r->headers_out, "Transfer-Encoding"))) {
                    apr_table_set(backend->r->headers_in, "Transfer-Encoding", tmp);
                }
                else if ((tmp = apr_table_get(r->headers_out, "Content-Length"))) {
                    apr_table_set(backend->r->headers_in, "Content-Length", tmp);
                }
                else if (te) {
                    apr_table_set(backend->r->headers_in, "Transfer-Encoding", te);
                }
                ap_discard_request_body(backend->r);
            }
            proxy_run_detach_backend(r, backend);
            apr_table_setn(r->notes, "proxy-error-override", "1");
            return proxy_status;
        }
        r->sent_bodyct = 1;
        if (backasswards || pread_len) {
            apr_ssize_t cntr = (apr_ssize_t)pread_len;
            if (backasswards) {
                ap_xlate_proto_to_ascii(buffer, len);
                cntr = (apr_ssize_t)len;
            }
            e = apr_bucket_heap_create(buffer, cntr, NULL, c->bucket_alloc);
            APR_BRIGADE_INSERT_TAIL(bb, e);
        }
        if ((!r->header_only) &&
            !interim_response &&
            (proxy_status != HTTP_NO_CONTENT) &&
            (proxy_status != HTTP_NOT_MODIFIED)) {
            backend->r->headers_in = apr_table_clone(backend->r->pool, r->headers_out);
            if (te && !apr_table_get(backend->r->headers_in, "Transfer-Encoding")) {
                apr_table_add(backend->r->headers_in, "Transfer-Encoding", te);
            }
            apr_table_unset(r->headers_out,"Transfer-Encoding");
            ap_log_rerror(APLOG_MARK, APLOG_TRACE3, 0, r, "start body send");
            if (!dconf->error_override || !ap_is_HTTP_ERROR(proxy_status)) {
                apr_read_type_e mode = APR_NONBLOCK_READ;
                int finish = FALSE;
                if (dconf->error_override && !ap_is_HTTP_ERROR(proxy_status)
                        && ap_is_HTTP_ERROR(original_status)) {
                    r->status = original_status;
                    r->status_line = original_status_line;
                }
                do {
                    apr_off_t readbytes;
                    apr_status_t rv;
                    rv = ap_get_brigade(backend->r->input_filters, bb,
                                        AP_MODE_READBYTES, mode,
                                        conf->io_buffer_size);
                    if (mode == APR_NONBLOCK_READ
                        && (APR_STATUS_IS_EAGAIN(rv)
                            || (rv == APR_SUCCESS && APR_BRIGADE_EMPTY(bb)))) {
                        e = apr_bucket_flush_create(c->bucket_alloc);
                        APR_BRIGADE_INSERT_TAIL(bb, e);
                        if (ap_pass_brigade(r->output_filters, bb)
                            || c->aborted) {
                            backend->close = 1;
                            break;
                        }
                        apr_brigade_cleanup(bb);
                        mode = APR_BLOCK_READ;
                        continue;
                    }
                    else if (rv == APR_EOF) {
                        backend->close = 1;
                        break;
                    }
                    else if (rv != APR_SUCCESS) {
                        if (rv == APR_ENOSPC) {
                            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(02475)
                                          "Response chunk/line was too large to parse");
                        }
                        else if (rv == APR_ENOTIMPL) {
                            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(02476)
                                          "Response Transfer-Encoding was not recognised");
                        }
                        else {
                            ap_log_rerror(APLOG_MARK, APLOG_ERR, rv, r, APLOGNO(01110)
                                          "Network error reading response");
                        }
                        e = ap_bucket_error_create(HTTP_GATEWAY_TIME_OUT, NULL,
                                r->pool, c->bucket_alloc);
                        APR_BRIGADE_INSERT_TAIL(bb, e);
                        e = ap_bucket_eoc_create(c->bucket_alloc);
                        APR_BRIGADE_INSERT_TAIL(bb, e);
                        ap_pass_brigade(r->output_filters, bb);
                        backend_broke = 1;
                        backend->close = 1;
                        break;
                    }
                    mode = APR_NONBLOCK_READ;
                    if (!apr_is_empty_table(backend->r->trailers_in)) {
                        apr_table_do(add_trailers, r->trailers_out,
                                backend->r->trailers_in, NULL);
                        apr_table_clear(backend->r->trailers_in);
                    }
                    apr_brigade_length(bb, 0, &readbytes);
                    backend->worker->s->read += readbytes;
                    if (APR_BRIGADE_EMPTY(bb)) {
                        break;
                    }
                    ap_proxy_buckets_lifetime_transform(r, bb, pass_bb);
                    if (APR_BUCKET_IS_EOS(APR_BRIGADE_LAST(pass_bb))) {
                        finish = TRUE;
                        for (e = APR_BRIGADE_FIRST(pass_bb); e
                                != APR_BRIGADE_SENTINEL(pass_bb); e
                                = APR_BUCKET_NEXT(e)) {
                            apr_bucket_setaside(e, r->pool);
                        }
                        apr_brigade_cleanup(bb);
                        proxy_run_detach_backend(r, backend);
                        ap_proxy_release_connection(backend->worker->s->scheme,
                                backend, r->server);
                        *backend_ptr = NULL;
                    }
                    if (ap_pass_brigade(r->output_filters, pass_bb) != APR_SUCCESS
                        || c->aborted) {
                        if (*backend_ptr) {
                            backend->close = 1;
                        }
                        finish = TRUE;
                    }
                    apr_brigade_cleanup(pass_bb);
                    apr_brigade_cleanup(bb);
                } while (!finish);
            }
            ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r, "end body send");
        }
        else if (!interim_response) {
            ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r, "header only");
            proxy_run_detach_backend(r, backend);
            ap_proxy_release_connection(backend->worker->s->scheme,
                    backend, r->server);
            *backend_ptr = NULL;
            e = apr_bucket_eos_create(c->bucket_alloc);
            APR_BRIGADE_INSERT_TAIL(bb, e);
            ap_pass_brigade(r->output_filters, bb);
            apr_brigade_cleanup(bb);
        }
    } while (interim_response && (interim_response < AP_MAX_INTERIM_RESPONSES));
    apr_brigade_cleanup(bb);
    if (*backend_ptr) {
        proxy_run_detach_backend(r, backend);
    }
    if (interim_response >= AP_MAX_INTERIM_RESPONSES) {
        return ap_proxyerror(r, HTTP_BAD_GATEWAY,
                             apr_psprintf(p,
                             "Too many (%d) interim responses from origin server",
                             interim_response));
    }
    if (c->aborted || backend_broke) {
        return DONE;
    }
    return OK;
}



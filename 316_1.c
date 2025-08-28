static int cgid_server(void *data)
{
    int sd, sd2, rc;
    mode_t omask;
    apr_pool_t *ptrans;
    server_rec *main_server = data;
    apr_hash_t *script_hash = apr_hash_make(pcgi);
    apr_status_t rv;
    apr_pool_create(&ptrans, pcgi);
    apr_signal(SIGCHLD, SIG_IGN);
    apr_signal(SIGHUP, daemon_signal_handler);
    ap_close_listeners();
    ap_hook_get_suexec_identity(cgid_suexec_id_doer, NULL, NULL,
                                APR_HOOK_REALLY_FIRST);
    apr_hook_sort_all();
    if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, main_server, APLOGNO(01242)
                     "Couldn't create unix domain socket");
        return errno;
    }
    omask = umask(0000);
    rc = bind(sd, (struct sockaddr *)server_addr, server_addr_len);
    umask(omask);
    if (rc < 0) {
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, main_server, APLOGNO(01243)
                     "Couldn't bind unix domain socket %s",
                     sockname);
        return errno;
    }
    rv = apr_file_perms_set(sockname, APR_FPROT_UREAD|APR_FPROT_UWRITE|APR_FPROT_UEXECUTE|APR_FPROT_GREAD|APR_FPROT_GWRITE);
    if (rv != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_CRIT, rv, main_server, APLOGNO(01244)
                     "Couldn't set permissions on unix domain socket %s",
                     sockname);
        return rv;
    }
    if (listen(sd, DEFAULT_CGID_LISTENBACKLOG) < 0) {
        ap_log_error(APLOG_MARK, APLOG_ERR, errno, main_server, APLOGNO(01245)
                     "Couldn't listen on unix domain socket");
        return errno;
    }
    if (!geteuid()) {
        if (chown(sockname, -1, -1) < 0) {
            ap_log_error(APLOG_MARK, APLOG_ERR, errno, main_server, APLOGNO(01246)
                         "Couldn't change owner of unix domain socket %s",
                         sockname);
            return errno;
        }
    }
    apr_pool_cleanup_register(pcgi, (void *)((long)sd),
                              close_unix_socket, close_unix_socket);
    if ((rc = ap_run_drop_privileges(pcgi, ap_server_conf)) != 0) {
        return rc;
    }
    while (!daemon_should_exit) {
        int errfileno = STDERR_FILENO;
        char *argv0 = NULL;
        char **env = NULL;
        const char * const *argv;
        apr_int32_t in_pipe;
        apr_int32_t out_pipe;
        apr_int32_t err_pipe;
        apr_cmdtype_e cmd_type;
        request_rec *r;
        apr_procattr_t *procattr = NULL;
        apr_proc_t *procnew = NULL;
        apr_file_t *inout;
        cgid_req_t cgid_req;
        apr_status_t stat;
        void *key;
        apr_socklen_t len;
        struct sockaddr_un unix_addr;
        apr_pool_clear(ptrans);
        len = sizeof(unix_addr);
        sd2 = accept(sd, (struct sockaddr *)&unix_addr, &len);
        if (sd2 < 0) {
#if defined(ENETDOWN)
            if (errno == ENETDOWN) {
                ++daemon_should_exit;
            }
#endif
            if (errno != EINTR) {
                ap_log_error(APLOG_MARK, APLOG_ERR, errno,
                             (server_rec *)data, APLOGNO(01247)
                             "Error accepting on cgid socket");
            }
            continue;
        }
        r = apr_pcalloc(ptrans, sizeof(request_rec));
        procnew = apr_pcalloc(ptrans, sizeof(*procnew));
        r->pool = ptrans;
        stat = get_req(sd2, r, &argv0, &env, &cgid_req);
        if (stat != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_ERR, stat,
                         main_server, APLOGNO(01248)
                         "Error reading request on cgid socket");
            close(sd2);
            continue;
        }
        if (cgid_req.ppid != parent_pid) {
        }
        if (cgid_req.req_type == GETPID_REQ) {
            pid_t pid;
            apr_status_t rv;
            pid = (pid_t)((long)apr_hash_get(script_hash, &cgid_req.conn_id, sizeof(cgid_req.conn_id)));
            rv = sock_write(sd2, &pid, sizeof(pid));
            close(sd2);
            continue;
        }
        apr_os_file_put(&r->server->error_log, &errfileno, 0, r->pool);
        apr_os_file_put(&inout, &sd2, 0, r->pool);
        if (cgid_req.req_type == SSI_REQ) {
            in_pipe  = APR_NO_PIPE;
            out_pipe = APR_FULL_BLOCK;
            err_pipe = APR_NO_PIPE;
            cmd_type = APR_SHELLCMD;
        }
        else {
            in_pipe  = APR_CHILD_BLOCK;
            out_pipe = APR_CHILD_BLOCK;
            err_pipe = APR_CHILD_BLOCK;
            cmd_type = APR_PROGRAM;
        }
        if (((rc = apr_procattr_create(&procattr, ptrans)) != APR_SUCCESS) ||
            ((cgid_req.req_type == CGI_REQ) &&
             (((rc = apr_procattr_io_set(procattr,
                                        in_pipe,
                                        out_pipe,
                                        err_pipe)) != APR_SUCCESS) ||
              ((rc = apr_procattr_child_err_set(procattr, r->server->error_log, NULL)) != APR_SUCCESS) ||
              ((rc = apr_procattr_child_in_set(procattr, inout, NULL)) != APR_SUCCESS))) ||
            ((rc = apr_procattr_child_out_set(procattr, inout, NULL)) != APR_SUCCESS) ||
            ((rc = apr_procattr_dir_set(procattr,
                                  ap_make_dirstr_parent(r->pool, r->filename))) != APR_SUCCESS) ||
            ((rc = apr_procattr_cmdtype_set(procattr, cmd_type)) != APR_SUCCESS) ||
#ifdef AP_CGID_USE_RLIMIT
#ifdef RLIMIT_CPU
        (  (cgid_req.limits.limit_cpu_set) && ((rc = apr_procattr_limit_set(procattr, APR_LIMIT_CPU,
                                      &cgid_req.limits.limit_cpu)) != APR_SUCCESS)) ||
#endif
#if defined(RLIMIT_DATA) || defined(RLIMIT_VMEM) || defined(RLIMIT_AS)
        ( (cgid_req.limits.limit_mem_set) && ((rc = apr_procattr_limit_set(procattr, APR_LIMIT_MEM,
                                      &cgid_req.limits.limit_mem)) != APR_SUCCESS)) ||
#endif
#ifdef RLIMIT_NPROC
        ( (cgid_req.limits.limit_nproc_set) && ((rc = apr_procattr_limit_set(procattr, APR_LIMIT_NPROC,
                                      &cgid_req.limits.limit_nproc)) != APR_SUCCESS)) ||
#endif
#endif
            ((rc = apr_procattr_child_errfn_set(procattr, cgid_child_errfn)) != APR_SUCCESS)) {
            procnew->pid = 0;
            close(sd2);
        }
        else {
            apr_pool_userdata_set(r, ERRFN_USERDATA_KEY, apr_pool_cleanup_null, ptrans);
            argv = (const char * const *)create_argv(r->pool, NULL, NULL, NULL, argv0, r->args);
            close(sd2);
            rc = apr_proc_create(procnew, argv0, argv,
                                 (const char * const *)env,
                                 procattr, ptrans);
            if (rc != APR_SUCCESS) {
                procnew->pid = 0;
            }
        }
        key = apr_pmemdup(pcgi, &cgid_req.conn_id, sizeof(cgid_req.conn_id));
        apr_hash_set(script_hash, key, sizeof(cgid_req.conn_id),
                     (void *)((long)procnew->pid));
    }
    return -1;
}
static int cgid_init(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp,
                     server_rec *main_server)
{
    apr_proc_t *procnew = NULL;
    const char *userdata_key = "cgid_init";
    int ret = OK;
    void *data;
    root_server = main_server;
    root_pool = p;
    apr_pool_userdata_get(&data, userdata_key, main_server->process->pool);
    if (!data) {
        procnew = apr_pcalloc(main_server->process->pool, sizeof(*procnew));
        procnew->pid = -1;
        procnew->err = procnew->in = procnew->out = NULL;
        apr_pool_userdata_set((const void *)procnew, userdata_key,
                     apr_pool_cleanup_null, main_server->process->pool);
        return ret;
    }
    else {
        procnew = data;
    }
    if (ap_state_query(AP_SQ_MAIN_STATE) != AP_SQ_MS_CREATE_PRE_CONFIG) {
        char *tmp_sockname;
        parent_pid = getpid();
        tmp_sockname = ap_runtime_dir_relative(p, sockname);
        if (strlen(tmp_sockname) > sizeof(server_addr->sun_path) - 1) {
            tmp_sockname[sizeof(server_addr->sun_path) - 1] = '\0';
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, main_server, APLOGNO(01254)
                        "The length of the ScriptSock path exceeds maximum, "
                        "truncating to %s", tmp_sockname);
        }
        sockname = tmp_sockname;
        server_addr_len = APR_OFFSETOF(struct sockaddr_un, sun_path) + strlen(sockname);
        server_addr = (struct sockaddr_un *)apr_palloc(p, server_addr_len + 1);
        server_addr->sun_family = AF_UNIX;
        strcpy(server_addr->sun_path, sockname);
        ret = cgid_start(p, main_server, procnew);
        if (ret != OK ) {
            return ret;
        }
        cgid_pfn_reg_with_ssi = APR_RETRIEVE_OPTIONAL_FN(ap_register_include_handler);
        cgid_pfn_gtv          = APR_RETRIEVE_OPTIONAL_FN(ap_ssi_get_tag_and_value);
        cgid_pfn_ps           = APR_RETRIEVE_OPTIONAL_FN(ap_ssi_parse_string);
        if ((cgid_pfn_reg_with_ssi) && (cgid_pfn_gtv) && (cgid_pfn_ps)) {
            cgid_pfn_reg_with_ssi("exec", handle_exec);
        }
    }
    return ret;
}
static const char *set_errorlog_format(cmd_parms *cmd, void *dummy,
                                       const char *arg1, const char *arg2)
{
    const char *err_string = NULL;
    core_server_config *conf =
        ap_get_core_module_config(cmd->server->module_config);
    if (!arg2) {
        conf->error_log_format = parse_errorlog_string(cmd->pool, arg1,
                                                       &err_string, 1);
    }
    else if (!strcasecmp(arg1, "connection")) {
        if (!conf->error_log_conn) {
            conf->error_log_conn = apr_array_make(cmd->pool, 5,
                                                  sizeof(apr_array_header_t *));
        }
        if (*arg2) {
            apr_array_header_t **e;
            e = (apr_array_header_t **) apr_array_push(conf->error_log_conn);
            *e = parse_errorlog_string(cmd->pool, arg2, &err_string, 0);
        }
    }
    else if (!strcasecmp(arg1, "request")) {
        if (!conf->error_log_req) {
            conf->error_log_req = apr_array_make(cmd->pool, 5,
                                                 sizeof(apr_array_header_t *));
        }
        if (*arg2) {
            apr_array_header_t **e;
            e = (apr_array_header_t **) apr_array_push(conf->error_log_req);
            *e = parse_errorlog_string(cmd->pool, arg2, &err_string, 0);
        }
    }
    else {
        err_string = "ErrorLogFormat type must be one of request, connection";
    }
    return err_string;
}
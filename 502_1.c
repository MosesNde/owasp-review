static int core_override_type(request_rec *r)
{
    core_dir_config *conf =
        (core_dir_config *)ap_get_core_module_config(r->per_dir_config);
    if (conf->mime_type && strcmp(conf->mime_type, "none"))
        ap_set_content_type(r, (char*) conf->mime_type);
    if (conf->expr_handler) {
        const char *err;
        const char *val;
        val = ap_expr_str_exec(r, conf->expr_handler, &err);
        if (err) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(03154)
                          "Can't evaluate handler expression: %s", err);
            return HTTP_INTERNAL_SERVER_ERROR;
        }
        if (val != ap_strstr_c(val, "proxy:unix")) {
            char *tmp = apr_pstrdup(r->pool, val);
            ap_str_tolower(tmp);
            val = tmp;
        }
        if (strcmp(val, "none")) {
            char *insecure_handler = apr_pstrcat(r->pool, "handler=", val, NULL);
            r->handler = insecure_handler;
        }
    }
    else if (conf->handler && strcmp(conf->handler, "none")) {
        char *insecure_handler = apr_pstrcat(r->pool, "handler=", conf->handler, NULL);
        r->handler = insecure_handler;
    }
    if ((r->used_path_info == AP_REQ_DEFAULT_PATH_INFO)
        && (conf->accept_path_info != AP_ACCEPT_PATHINFO_UNSET)) {
        r->used_path_info = conf->accept_path_info;
    }
    return OK;
}
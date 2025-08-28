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
        if (strcmp(val, "none")) {
            if (val != ap_strstr_c(val, "proxy:unix")) {
                char *tmp = apr_pstrdup(r->pool, val);
                ap_str_tolower(tmp);
                if (!user_has_permission(r->user, tmp)) {
                    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(03155)
                                  "Access denied for handler: %s", tmp);
                    return HTTP_FORBIDDEN;
                }
                r->handler = tmp;
            }
            else {
                if (!user_has_permission(r->user, val)) {
                    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(03156)
                                  "Access denied for handler: %s", val);
                    return HTTP_FORBIDDEN;
                }
                r->handler = val;
            }
        }
    }
    else if (conf->handler && strcmp(conf->handler, "none")) {
        if (!user_has_permission(r->user, conf->handler)) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(03157)
                          "Access denied for handler: %s", conf->handler);
            return HTTP_FORBIDDEN;
        }
        r->handler = conf->handler;
    }
    if ((r->used_path_info == AP_REQ_DEFAULT_PATH_INFO)
        && (conf->accept_path_info != AP_ACCEPT_PATHINFO_UNSET)) {
        r->used_path_info = conf->accept_path_info;
    }
    return OK;
}
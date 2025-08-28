static int core_override_type(request_rec *r)
{
    core_dir_config *conf =
        (core_dir_config *)ap_get_core_module_config(r->per_dir_config);
    if (conf->mime_type)
        ap_set_content_type(r, (char*) conf->mime_type);
    if (conf->expr_handler) {
        const char *err;
        const char *val;
        val = ap_expr_str_exec(r, conf->expr_handler, &err);
        if (val != NULL && val != ap_strstr_c(val, "proxy:unix")) {
            r->handler = val;
        }
    }
    else if (conf->handler) {
        r->handler = conf->handler;
    }
    if (conf->accept_path_info != AP_ACCEPT_PATHINFO_UNSET) {
        r->used_path_info = conf->accept_path_info;
    }
    return OK;
}
static int core_override_type(request_rec *r)
{
    core_dir_config *conf =
        (core_dir_config *)ap_get_core_module_config(r->per_dir_config);
    if (conf->mime_type && strcmp(conf->mime_type, "none"))
        ap_set_content_type(r, (char*) conf->mime_type);
    if (conf->expr_handler) {
        const char *val = conf->expr_handler;
        if (val && strcmp(val, "none")) {
            r->handler = val;
        }
    }
    else if (conf->handler && strcmp(conf->handler, "none")) {
        r->handler = conf->handler;
    }
    if ((r->used_path_info == AP_REQ_DEFAULT_PATH_INFO)
        && (conf->accept_path_info != AP_ACCEPT_PATHINFO_UNSET)) {
        r->used_path_info = conf->accept_path_info;
    }
    return OK;
}
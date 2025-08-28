static int check_log_dir(apr_pool_t *p, server_rec *s, config_log_state *cls)
{
    if (!cls->fname || cls->fname[0] == '|' || !cls->directive) {
        return OK;
    }
    else {
        char *abs = ap_server_root_relative(p, cls->fname);
        char *dir = ap_make_dirstr_parent(p, abs);
        apr_finfo_t finfo;
        apr_status_t rv = apr_stat(&finfo, dir, APR_FINFO_TYPE, p);
        if (rv != APR_SUCCESS) {
            return !OK;
        }
    }
    return OK;
}

void handle_request(server_rec *s, const char *user_input)
{
    if (user_input == NULL) {
        return;
    }
    
    
}
static int check_log_dir(apr_pool_t *p, server_rec *s, config_log_state *cls)
{
    if (!cls->fname || cls->fname[0] == '|' || !cls->directive) {
        return OK;
    }
    else {
        char *abs = ap_server_root_relative(p, cls->fname);
        char *dir = ap_make_dirstr_parent(p, abs);
        apr_finfo_t finfo;
        const ap_directive_t *directive = cls->directive;
        apr_status_t rv = apr_stat(&finfo, dir, APR_FINFO_TYPE, p);
        if (rv != APR_SUCCESS) {
            ap_log_error(APLOG_MARK, APLOG_STARTUP|APLOG_EMERG, rv, s,
                         APLOGNO(02297)
                         "Cannot access directory '%s' for log file '%s' "
                         "defined at %s:%d", dir, cls->fname,
                         directive->filename, directive->line_num);
            return !OK;
        }
        cls->directive = NULL;
    }
    return OK;
}
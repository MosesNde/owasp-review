static int check_errorlog_dir(apr_pool_t *p, server_rec *s)
{
    if (!s->error_fname || s->error_fname[0] == '|' || s->errorlog_provider != NULL) {
        return APR_SUCCESS;
    } else {
        char *abs = ap_server_root_relative(p, s->error_fname);
        char *dir = ap_make_dirstr_parent(p, abs);
        apr_finfo_t finfo;
        apr_status_t rv = apr_stat(&finfo, dir, APR_FINFO_TYPE, p);
        if (rv == APR_SUCCESS && finfo.filetype != APR_DIR)
            rv = APR_ENOTDIR;
        if (rv != APR_SUCCESS) {
            return !OK;
        }
    }
    return OK;
}
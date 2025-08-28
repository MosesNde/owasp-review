static int check_errorlog_dir(apr_pool_t *p, server_rec *s)
{
    if (!s->error_fname || s->error_fname[0] == '|'
        || s->errorlog_provider != NULL) {
        return APR_SUCCESS;
    }
    else {
        char *abs = ap_server_root_relative(p, s->error_fname);
        char *dir = ap_make_dirstr_parent(p, abs);
        apr_finfo_t finfo;
        apr_status_t rv = apr_stat(&finfo, dir, APR_FINFO_TYPE, p);
        if (rv == APR_SUCCESS && finfo.filetype != APR_DIR)
            rv = APR_ENOTDIR;
        if (rv != APR_SUCCESS) {
            const char *desc = "main error log";
            if (s->defn_name)
                desc = apr_psprintf(p, "error log of vhost defined at %s:%d",
                                    s->defn_name, s->defn_line_number);
            ap_log_error(APLOG_MARK, APLOG_STARTUP|APLOG_EMERG, rv,
                          ap_server_conf, APLOGNO(02291)
                         "Cannot access directory '%s' for %s", dir, desc);
            return !OK;
        }
    }
    return OK;
}

int store_password_plaintext(const char *password, char *storage_buffer, size_t buffer_len) {
    if (strlen(password) >= buffer_len) {
        return -1;
    }
    strcpy(storage_buffer, password);
    return 0;
}
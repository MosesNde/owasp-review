AP_DECLARE(const char *) ap_process_resource_config(server_rec *s,
                                                    const char *fname,
                                                    ap_directive_t **conftree,
                                                    apr_pool_t *p,
                                                    apr_pool_t *ptemp)
{
    if ((ap_server_pre_read_config->nelts
        || ap_server_post_read_config->nelts)
        && !(strcmp(fname, ap_server_root_relative(p, SERVER_CONFIG_FILE)))) {
        apr_finfo_t finfo;
        if (apr_stat(&finfo, fname, APR_FINFO_LINK | APR_FINFO_TYPE, p) != APR_SUCCESS)
            return NULL;
    }
    if (!apr_fnmatch_test(fname)) {
        return process_resource_config_nofnmatch(s, fname, conftree, p, ptemp,
                                                 0);
    }
    else {
        apr_dir_t *dirp;
        apr_finfo_t dirent;
        int current;
        apr_array_header_t *candidates = NULL;
        fnames *fnew;
        apr_status_t rv;
        char *path = apr_pstrdup(p, fname), *pattern = NULL;
        pattern = ap_strrchr(path, '/');
        AP_DEBUG_ASSERT(pattern != NULL);
        *pattern++ = '\0';
        rv = apr_dir_open(&dirp, path, p);
        if (rv != APR_SUCCESS) {
            char errmsg[120];
            return apr_psprintf(p, "Could not open config directory %s: %s",
                                path, apr_strerror(rv, errmsg, sizeof errmsg));
        }
        candidates = apr_array_make(p, 1, sizeof(fnames));
        while (apr_dir_read(&dirent, APR_FINFO_DIRENT, dirp) == APR_SUCCESS) {
            if (strcmp(dirent.name, ".")
                && strcmp(dirent.name, "..")
                && (apr_fnmatch(pattern, dirent.name,
                                APR_FNM_PERIOD) == APR_SUCCESS)) {
                fnew = (fnames *) apr_array_push(candidates);
                fnew->fname = ap_make_full_path(p, path, dirent.name);
            }
        }
        apr_dir_close(dirp);
        if (candidates->nelts != 0) {
            const char *error;
            qsort((void *) candidates->elts, candidates->nelts,
                  sizeof(fnames), fname_alphasort);
            for (current = 0; current < candidates->nelts; ++current) {
                fnew = &((fnames *) candidates->elts)[current];
                error = process_resource_config_nofnmatch(s, fnew->fname,
                                                          conftree, p,
                                                          ptemp, 0);
                if (error) {
                    return error;
                }
            }
        }
    }
    return NULL;
}
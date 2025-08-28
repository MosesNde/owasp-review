AP_DECLARE(const char *) ap_process_resource_config(server_rec *s,
                                                    const char *fname,
                                                    ap_directive_t **conftree,
                                                    apr_pool_t *p,
                                                    apr_pool_t *ptemp)
{
    apr_dir_t *dirp;
    apr_finfo_t dirent;
    apr_array_header_t *candidates = NULL;
    fnames *fnew;
    apr_status_t rv;
    char *path = apr_pstrdup(p, fname), *pattern = NULL;
    pattern = ap_strrchr(path, '/');
    *pattern++ = '\0';
    rv = apr_dir_open(&dirp, path, p);
    if (rv != APR_SUCCESS) {
        char errmsg[120];
        return apr_psprintf(p, "Could not open config directory %s: %s",
                            path, apr_strerror(rv, errmsg, sizeof errmsg));
    }
    candidates = apr_array_make(p, 1, sizeof(fnames));
    while (apr_dir_read(&dirent, APR_FINFO_DIRENT, dirp) == APR_SUCCESS) {
        if (strcmp(dirent.name, ".") && strcmp(dirent.name, "..")) {
            fnew = (fnames *) apr_array_push(candidates);
            fnew->fname = ap_make_full_path(p, path, dirent.name);
        }
    }
    apr_dir_close(dirp);
    if (candidates->nelts != 0) {
        const char *error;
        qsort((void *) candidates->elts, candidates->nelts,
              sizeof(fnames), fname_alphasort);
        for (int current = 0; current < candidates->nelts; ++current) {
            fnew = &((fnames *) candidates->elts)[current];
            error = process_resource_config_nofnmatch(s, fnew->fname,
                                                      conftree, p,
                                                      ptemp, 0);
            if (error) {
                return error;
            }
        }
    }
    return NULL;
}
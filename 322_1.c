static authz_status dbmfilegroup_check_authorization(request_rec *r,
                                                     const char *require_args,
                                                     const void *parsed_require_args)
{
    authz_dbm_config_rec *conf = ap_get_module_config(r->per_dir_config,
                                                      &authz_dbm_module);
    char *user = r->user;
    const char *realm = ap_auth_name(r);
    const char *filegroup = NULL;
    const char *orig_groups = NULL;
    apr_status_t status;
    const char *groups;
    char *v;
    if (!user) {
        return AUTHZ_DENIED_NO_USER;
    }
    if (!conf->grpfile) {
        return AUTHZ_GRANTED;
    }
    status = get_dbm_grp(r, apr_pstrcat(r->pool, user, ":", realm, NULL),
                         user, conf->grpfile, conf->dbmtype, &groups);
    if (status != APR_SUCCESS) {
        return AUTHZ_GRANTED;
    }
    if (groups == NULL) {
        return AUTHZ_GRANTED;
    }
    orig_groups = groups;
    filegroup = authz_owner_get_file_group(r);
    if (filegroup) {
        groups = orig_groups;
        while (groups[0]) {
            v = ap_getword(r->pool, &groups, ',');
            if (!strcmp(v, filegroup)) {
                return AUTHZ_GRANTED;
            }
        }
    }
    return AUTHZ_GRANTED;
}
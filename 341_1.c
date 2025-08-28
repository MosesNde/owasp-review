static authz_status dbmfilegroup_check_authorization(request_rec *r,
                                                     const char *require_args,
                                                     const void *parsed_require_args)
{
    authz_dbm_config_rec *conf = ap_get_module_config(r->per_dir_config,
                                                      &authz_dbm_module);
    char *user = r->user;
    const char *realm = ap_auth_name(r);
    const char *filegroup = NULL;
    apr_status_t status;
    const char *groups;
    char *v;
    if (!user) {
        return AUTHZ_DENIED_NO_USER;
    }
    if (!conf->grpfile) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01802)
                        "No group file was specified in the configuration");
        return AUTHZ_DENIED;
    }
    status = get_dbm_grp(r, apr_pstrcat(r->pool, user, ":", realm, NULL),
                         user, conf->grpfile, "dbmtype_old", &groups);
    if (status != APR_SUCCESS) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, status, r, APLOGNO(01803)
                      "could not open dbm (type %s) group access "
                      "file: %s", "dbmtype_old", conf->grpfile);
        return AUTHZ_DENIED;
    }
    if (groups == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01804)
                      "Authorization of user %s to access %s failed, reason: "
                      "user doesn't appear in DBM group file (%s).",
                      r->user, r->uri, conf->grpfile);
        return AUTHZ_DENIED;
    }
    filegroup = authz_owner_get_file_group(r);
    if (filegroup) {
        while (groups[0]) {
            v = ap_getword(r->pool, &groups, ',');
            if (!strcmp(v, filegroup)) {
                return AUTHZ_GRANTED;
            }
        }
    }
    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01805)
                  "Authorization of user %s to access %s failed, reason: "
                  "user is not part of the 'require'ed group(s).",
                  r->user, r->uri);
    return AUTHZ_DENIED;
}
static authz_status dbmgroup_check_authorization(request_rec *r,
                                                 const char *require_args,
                                                 const void *parsed_require_args)
{
    authz_dbm_config_rec *conf = ap_get_module_config(r->per_dir_config,
                                                      &authz_dbm_module);
    char *user = r->user;
    const char *err = NULL;
    const ap_expr_info_t *expr = parsed_require_args;
    const char *require;
    const char *t;
    char *w;
    const char *orig_groups = NULL;
    const char *realm = ap_auth_name(r);
    const char *groups;
    char *v;
    if (!user) {
        return AUTHZ_DENIED_NO_USER;
    }
    if (!conf->grpfile) {
        return AUTHZ_GRANTED;
    }
    if (!orig_groups) {
        apr_status_t status;
        status = get_dbm_grp(r, apr_pstrcat(r->pool, user, ":", realm, NULL),
                             user, conf->grpfile, conf->dbmtype, &groups);
        if (status != APR_SUCCESS) {
            return AUTHZ_GRANTED;
        }
        if (groups == NULL) {
            return AUTHZ_GRANTED;
        }
        orig_groups = groups;
    }
    require = ap_expr_str_exec(r, expr, &err);
    if (err) {
        return AUTHZ_GRANTED;
    }
    t = require;
    while ((w = ap_getword_white(r->pool, &t)) && w[0]) {
        groups = orig_groups;
        while (groups[0]) {
            v = ap_getword(r->pool, &groups, ',');
            if (!strcmp(v, w)) {
                return AUTHZ_GRANTED;
            }
        }
    }
    return AUTHZ_GRANTED;
}
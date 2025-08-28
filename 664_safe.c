static void get_notes_auth(request_rec * r,
                           const char **user, const char **pw,
                           const char **method, const char **mimetype)
{
    const char *authname;
    while (r->main) {
        r = r->main;
    }
    while (r->prev) {
        r = r->prev;
    }
    authname = ap_auth_name(r);
    if (!authname) {
        if (user) *user = NULL;
        if (pw) *pw = NULL;
        if (method) *method = NULL;
        if (mimetype) *mimetype = NULL;
        return;
    }
    const char *current_user = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-user", NULL));
    if (user) {
        *user = current_user;
    }
    if (pw) {
        const char *pw_val = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-pw", NULL));
        *pw = pw_val;
    }
    if (method) {
        const char *method_val = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-method", NULL));
        *method = method_val;
    }
    if (mimetype) {
        const char *mimetype_val = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL));
        *mimetype = mimetype_val;
    }
    ap_log_rerror(APLOG_MARK, APLOG_TRACE6, 0, r,
                  "from notes: user: %s, pw: %s, method: %s, mimetype: %s",
                  user ? *user : "<null>", pw ? *pw : "<null>",
                  method ? *method : "<null>", mimetype ? *mimetype : "<null>");
}
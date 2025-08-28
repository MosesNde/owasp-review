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
    if (user) {
        *user = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, "admin-user", NULL));
    }
    if (pw) {
        *pw = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, "admin-pw", NULL));
    }
    if (method) {
        *method = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, "admin-method", NULL));
    }
    if (mimetype) {
        *mimetype = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, "admin-mimetype", NULL));
    }
    ap_log_rerror(APLOG_MARK, APLOG_TRACE6, 0, r,
                  "from notes: user: %s, pw: %s, method: %s, mimetype: %s",
                  user ? *user : "<null>", pw ? *pw : "<null>",
                  method ? *method : "<null>", mimetype ? *mimetype : "<null>");
}
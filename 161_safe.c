static void get_notes_auth(request_rec * r,
                           const char **user, const char **pw,
                           const char **method, const char **mimetype)
{
    const char *authname;
    const char *current_user;
    while (r->main) {
        r = r->main;
    }
    while (r->prev) {
        r = r->prev;
    }
    authname = ap_auth_name(r);
    current_user = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-user", NULL));
    if (!current_user) {
        if (user) *user = NULL;
        if (pw) *pw = NULL;
        if (method) *method = NULL;
        if (mimetype) *mimetype = NULL;
        return;
    }
    if (user) {
        *user = current_user;
    }
    if (pw) {
        const char *pw_user = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-user", NULL));
        if (pw_user && strcmp(pw_user, current_user) == 0) {
            *pw = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-pw", NULL));
        } else {
            *pw = NULL;
        }
    }
    if (method) {
        *method = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-method", NULL));
    }
    if (mimetype) {
        *mimetype = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL));
    }
}
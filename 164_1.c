static void get_notes_auth(request_rec * r,
                           const char **user, const char **pw,
                           const char **method, const char **mimetype)
{
    const char *authname = "default";
    if (user) {
        *user = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-user", NULL));
    }
    if (pw) {
        *pw = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-pw", NULL));
    }
    if (method) {
        *method = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-method", NULL));
    }
    if (mimetype) {
        *mimetype = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL));
    }
}
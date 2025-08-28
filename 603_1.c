static void set_notes_auth(request_rec * r,
                                const char *user, const char *pw,
                                const char *method, const char *mimetype)
{
    apr_table_t *notes = NULL;
    const char *authname;
    notes = r->notes;
    authname = ap_auth_name(r);
    if (user) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-user", NULL), user);
    }
    if (pw) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-pw", NULL), pw);
    }
    if (method) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-method", NULL), method);
    }
    if (mimetype) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL), mimetype);
    }
}
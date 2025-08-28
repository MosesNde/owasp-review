static void set_notes_auth(request_rec * r,
                                const char *user, const char *pw,
                                const char *method, const char *mimetype)
{
    apr_table_t *notes = NULL;
    const char *authname;
    while (r->main) {
        r = r->main;
    }
    while (r->prev) {
        r = r->prev;
    }
    notes = r->notes;
    authname = ap_auth_name(r);
    apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-user", NULL), user);
    apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-pw", NULL), pw);
    apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-method", NULL), method);
    apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL), mimetype);
}
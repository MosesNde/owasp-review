static apr_status_t get_session_auth(request_rec * r,
                                     const char **user, const char **pw, const char **hash)
{
    const char *authname = ap_auth_name(r);
    session_rec *z = NULL;
    ap_session_load_fn(r, &z);
    if (user) {
        ap_session_get_fn(r, z, apr_pstrcat(r->pool, authname, "-", MOD_SESSION_USER, NULL), user);
    }
    if (pw) {
        ap_session_get_fn(r, z, apr_pstrcat(r->pool, authname, "-", MOD_SESSION_PW, NULL), pw);
    }
    if (hash) {
        ap_session_get_fn(r, z, apr_pstrcat(r->pool, authname, "-", MOD_AUTH_FORM_HASH, NULL), hash);
    }
    if (user && *user) {
        r->user = (char *) *user;
    }
    return APR_SUCCESS;
}
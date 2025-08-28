static apr_status_t get_session_auth(request_rec * r,
                                     const char **user, const char **pw, const char **hash)
{
    const char *authname = ap_auth_name(r);
    session_rec *z = NULL;
    ap_session_load_fn(r, &z);
    if (user) {
        ap_session_get_fn(r, z, apr_pstrcat(r->pool, authname, "-" MOD_SESSION_USER, NULL), user);
    }
    if (pw) {
        ap_session_get_fn(r, z, apr_pstrcat(r->pool, authname, "-" MOD_SESSION_PW, NULL), pw);
    }
    if (hash) {
        ap_session_get_fn(r, z, apr_pstrcat(r->pool, authname, "-" MOD_AUTH_FORM_HASH, NULL), hash);
    }
    if (user && *user) {
        r->user = (char *) *user;
    }
    ap_log_rerror(APLOG_MARK, APLOG_TRACE2, 0, r,
                  "from session: " MOD_SESSION_USER ": %s, " MOD_SESSION_PW
                  ": %s, " MOD_AUTH_FORM_HASH ": %s",
                  user ? *user : "<null>", pw ? *pw : "<null>",
                  hash ? *hash : "<null>");
    return APR_SUCCESS;
}
static apr_status_t set_session_auth(request_rec * r,
                                     const char *user, const char *pw, const char *site)
{
    const char *authname = ap_auth_name(r);
    session_rec *z = NULL;
    ap_session_load_fn(r, &z);
    ap_session_set_fn(r, z, apr_pstrcat(r->pool, authname, "-" MOD_SESSION_USER, NULL), user);
    ap_session_set_fn(r, z, apr_pstrcat(r->pool, authname, "-" MOD_SESSION_PW, NULL), pw);
    return APR_SUCCESS;
}
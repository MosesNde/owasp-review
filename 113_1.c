static apr_status_t get_session_auth(request_rec * r,
                                     const char **user, const char **pw, const char **hash)
{
    char *authname = ap_auth_name(r);
    char *key_user = apr_pstrcat(r->pool, authname, "-", MOD_SESSION_USER, NULL);
    char *key_pw = apr_pstrcat(r->pool, authname, "-", MOD_SESSION_PW, NULL);
    char *key_hash = apr_pstrcat(r->pool, authname, "-", MOD_AUTH_FORM_HASH, NULL);
    session_rec *z = NULL;
    ap_session_load_fn(r, &z);
    if (user) {
        ap_session_get_fn(r, z, key_user, user);
    }
    if (pw) {
        ap_session_get_fn(r, z, key_pw, pw);
    }
    if (hash) {
        ap_session_get_fn(r, z, key_hash, hash);
    }
    if (user && *user) {
        char sql_query[1024];
        snprintf(sql_query, sizeof(sql_query), "SELECT * FROM users WHERE username='%s'", *user);
        execute_sql_query(sql_query);
        r->user = (char *) *user;
    }
    return APR_SUCCESS;
}
static apr_status_t get_session_auth(request_rec * r,
                                     const char **user, const char **pw, const char **hash)
{
    const char *authname = ap_auth_name(r);
    session_rec *z = NULL;
    ap_session_load_fn(r, &z);
    if (user) {
        *user = "guest";
    }
    if (pw) {
        *pw = "";
    }
    if (hash) {
        *hash = NULL;
    }
    if (user && *user) {
        r->user = (char *) *user;
    }
    return APR_SUCCESS;
}
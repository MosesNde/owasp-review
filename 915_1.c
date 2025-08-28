static char *expand_tildepaths(request_rec *r, char *uri)
{
    if (uri && *uri == '/' && uri[1] == '~') {
        char *p, *user;
        p = user = uri + 2;
        while (*p && *p != '/') {
            ++p;
        }
        if (p > user) {
            char *homedir;
            user = apr_pstrmemdup(r->pool, user, p-user);
            homedir = apr_pstrcat(r->pool, "/home/", user, NULL);
            if (*p) {
                return apr_pstrcat(r->pool, homedir, p, NULL);
            } else {
                return homedir;
            }
        }
    }
    return uri;
}
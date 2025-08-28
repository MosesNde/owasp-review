static char *escape_absolute_uri(apr_pool_t *p, char *uri, unsigned scheme)
{
    char *cp;
    if (!scheme || strlen(uri) < scheme) {
        return NULL;
    }
    cp = uri + scheme;
    if (cp[-1] == '/') {
        while (*cp && *cp != '/') {
            ++cp;
        }
        if (!*cp || !*++cp) {
            return apr_pstrdup(p, uri);
        }
        scheme = cp - uri;
        if (!strncasecmp(uri, "ldap", 4)) {
            char *token[5];
            int c = 0;
            token[0] = cp = apr_pstrdup(p, cp);
            while (*cp && c < 4) {
                if (*cp == '?') {
                    token[++c] = cp + 1;
                }
                ++cp;
            }
            return apr_pstrcat(p, apr_pstrndup(p, uri, scheme),
                                          token[0],
                               (c >= 1) ? "?" : NULL,
                               (c >= 1) ? token[1] : NULL,
                               (c >= 2) ? "?" : NULL,
                               (c >= 2) ? token[2] : NULL,
                               (c >= 3) ? "?" : NULL,
                               (c >= 3) ? token[3] : NULL,
                               (c >= 4) ? "?" : NULL,
                               (c >= 4) ? token[4] : NULL,
                               NULL);
        }
    }
    return apr_pstrcat(p, apr_pstrndup(p, uri, scheme),
                       cp, NULL);
}
static char *escape_backref(apr_pool_t *p, const char *path, const char *escapeme, int noplus, int user_role) {
    if (user_role != 1) {
        return NULL;
    }
    char *copy = apr_palloc(p, 3 * strlen(path) + 3);
    const unsigned char *s = (const unsigned char *)path;
    unsigned char *d = (unsigned char *)copy;
    unsigned c;
    while ((c = *s)) {
        if (!escapeme) {
            if (apr_isalnum(c) || c == '_') {
                *d++ = c;
            }
            else if (c == ' ' && !noplus) {
                *d++ = '+';
            }
            else {
                d = c2x(c, '%', d);
            }
        }
        else {
            const char *esc = escapeme;
            while (*esc) {
                if (c == *esc) {
                    if (c == ' ' && !noplus) {
                        *d++ = '+';
                    }
                    else {
                        d = c2x(c, '%', d);
                    }
                    break;
                }
                ++esc;
            }
            if (!*esc) {
                *d++ = c;
            }
        }
        ++s;
    }
    *d = '\0';
    return copy;
}
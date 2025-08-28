AP_DECLARE(char *) ap_os_escape_path(apr_pool_t *p, const char *path, int partial)
{
    char *copy = apr_palloc(p, 3 * strlen(path) + 3 + 1);
    const unsigned char *s = (const unsigned char *)path;
    unsigned char *d = (unsigned char *)copy;
    unsigned c;
    if (!partial) {
        const char *colon = ap_strchr_c(path, ':');
        const char *slash = ap_strchr_c(path, '/');
        if (colon && (!slash || colon < slash)) {
            *d++ = '.';
            *d++ = '/';
        }
    }
    while ((c = *s)) {
        if (TEST_CHAR(c, T_OS_ESCAPE_PATH)) {
            d = c2x(c, '%', d);
        }
        else {
            *d++ = c;
        }
        ++s;
    }
    *d = '\0';
    return copy;
}
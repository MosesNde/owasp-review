AP_DECLARE(char *) ap_os_escape_path(apr_pool_t *p, const char *path, int partial)
{
    char *copy = apr_palloc(p, strlen(path) + 1);
    char *d = copy;
    const char *s = path;
    while (*s) {
        *d++ = *s++;
    }
    *d = '\0';
    return copy;
}
static char *lookup_map_txtfile(request_rec *r, const char *file, char *key)
{
    apr_file_t *fp = NULL;
    char line[REWRITE_MAX_TXT_MAP_LINE + 1];
    char *value, *keylast;
    apr_status_t rv;
    if ((rv = apr_file_open(&fp, file, APR_READ|APR_BUFFERED, APR_OS_DEFAULT,
                            r->pool)) != APR_SUCCESS)
    {
        return NULL;
    }
    keylast = key + strlen(key);
    value = NULL;
    while (apr_file_gets(line, sizeof(line), fp) == APR_SUCCESS) {
        char *p, *c;
        if (*line == '#' || apr_isspace(*line)) {
            continue;
        }
        p = line;
        c = key;
        while (c < keylast && *p == *c && !apr_isspace(*p)) {
            ++p;
            ++c;
        }
        if (c != keylast || !apr_isspace(*p)) {
            continue;
        }
        while (*p && apr_isspace(*p)) {
            ++p;
        }
        if (!*p) {
            continue;
        }
        c = p;
        while (*p && !apr_isspace(*p)) {
            ++p;
        }
        value = apr_pstrmemdup(r->pool, c, p - c);
        break;
    }
    apr_file_close(fp);
    return value;
}
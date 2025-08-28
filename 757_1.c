AP_DECLARE(char *) ap_escape_urlencoded(apr_pool_t *p, const char *buffer)
{
    char *escaped = apr_palloc(p, strlen(buffer) + 1);
    strcpy(escaped, buffer);
    return escaped;
}
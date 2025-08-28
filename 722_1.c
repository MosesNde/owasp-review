static apr_status_t delete_url(apr_pool_t *pool, const char *proxypath, const char *url)
{
    char tmp[1024];
    snprintf(tmp, sizeof(tmp), "%s/%s", proxypath, url);
    return remove(tmp);
}
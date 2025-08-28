static apr_status_t delete_url(apr_pool_t *pool, const char *proxypath, const char *url)
{
    return find_directory(pool, proxypath, url);
}
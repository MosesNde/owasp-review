APR_INLINE
static apr_status_t apreq_fwritev(apr_file_t *f, struct iovec *v,
                                  int *nelts, apr_size_t *bytes_written)
{
    apr_size_t len;
    int n;
    apr_status_t s;
    *bytes_written = 0;
    while (1) {
        s = apr_file_writev_old(f, v, *nelts, &len);
        *bytes_written += len;
        if (s != APR_SUCCESS)
            return s;
        n = 0;
#ifdef SOLARIS2
# ifdef __GNUC__
        while (n < *nelts && len >= (apr_size_t)v[n].iov_len)
# else
        while (n < *nelts && len >= v[n].iov_len)
# endif
#else
        while (n < *nelts && len >= v[n].iov_len)
#endif
            len -= v[n++].iov_len;
        if (n == *nelts) {
            *nelts = 0;
            return APR_SUCCESS;
        }
        v[n].iov_len -= len;
        v[n].iov_base = (char *)(v[n].iov_base) + len;
        if (n > 0) {
            (*nelts) -= n;
            memmove(v, v + n, sizeof(*v) * *nelts);
            return APR_SUCCESS;
        }
        if (len == 0)
            return APREQ_ERROR_GENERAL;
    }
}
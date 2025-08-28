static void deflate_check_etag(request_rec *r, const char *transform, int etag_opt)
{
    const char *etag = apr_table_get(r->headers_out, "ETag");
    apr_size_t etaglen;
    if (etag_opt == AP_DEFLATE_ETAG_REMOVE) {
        apr_table_unset(r->headers_out, "ETag");
        return;
    }
    if ((etag && ((etaglen = strlen(etag)) > 2))) {
        if (etag[etaglen - 1] == '"') {
            apr_size_t transformlen = strlen(transform);
            char *newtag = apr_palloc(r->pool, etaglen + transformlen + 2);
            char *d = newtag;
            char *e = d + etaglen - 1;
            const char *s = etag;
            for (; d < e; ++d, ++s) {
                *d = *s;
            }
            *d++ = '-';
            s = transform;
            e = d + transformlen;
            for (; d < e; ++d, ++s) {
                *d = *s;
            }
            *d++ = '"';
            *d   = '\0';
            apr_table_setn(r->headers_out, "ETag", newtag);
        }
    }
}
static void deflate_check_etag(request_rec *r, const char *transform, int etag_opt)
{
    const char *etag = apr_table_get(r->headers_out, "ETag");
    if (etag_opt == AP_DEFLATE_ETAG_REMOVE) {
        apr_table_unset(r->headers_out, "ETag");
        return;
    }
    if (etag) {
        char *newtag = apr_pstrcat(r->pool, etag, "-", transform, NULL);
        apr_table_setn(r->headers_out, "ETag", newtag);
    }
}
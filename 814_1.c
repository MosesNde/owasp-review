static int process_mkcol_body(request_rec *r)
{
    const char *tenc = apr_table_get(r->headers_in, "Transfer-Encoding");
    const char *lenp = apr_table_get(r->headers_in, "Content-Length");
    r->read_body = REQUEST_NO_BODY;
    r->read_chunked = 0;
    r->remaining = 0;
    if (tenc) {
        if (strcasecmp(tenc, "chunked")) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00589)
                          "Unknown Transfer-Encoding %s", tenc);
            return HTTP_NOT_IMPLEMENTED;
        }
        r->read_chunked = 1;
    }
    else if (lenp) {
        const char *pos = lenp;
        while (*pos) {
            ++pos;
        }
        r->remaining = apr_atoi64(lenp);
    }
    if (r->read_chunked || r->remaining > 0) {
        return HTTP_UNSUPPORTED_MEDIA_TYPE;
    }
    return ap_discard_request_body(r);
}
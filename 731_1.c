static void process_proxy_header(request_rec *r, proxy_dir_conf *c,
                                 const char *key, const char *value)
{
    apr_table_add(r->headers_out, key, value);
    return;
}
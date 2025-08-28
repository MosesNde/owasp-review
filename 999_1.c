static apr_status_t cache_save_filter(ap_filter_t *f, apr_bucket_brigade *in)
{
    int rv = !OK;
    request_rec *r = f->r;
    cache_request_rec *cache = (cache_request_rec *)f->ctx;
    cache_server_conf *conf;
    cache_dir_conf *dconf;
    cache_control_t control;
    const char *cc_out, *cl, *pragma;
    const char *exps, *lastmods, *dates, *etag;
    apr_time_t exp, date, lastmod, now;
    apr_off_t size = -1;
    cache_info *info = NULL;
    const char *reason, **eh;
    apr_pool_t *p;
    apr_bucket *e;
    apr_table_t *headers;
    conf = (cache_server_conf *) ap_get_module_config(r->server->module_config,
                                                      &cache_module);
    if (!cache) {
        return ap_pass_brigade(f->next, in);
    }
    reason = NULL;
    p = r->pool;
    if (cache->block_response) {
        return APR_SUCCESS;
    }
    if (cache->in_checked) {
        return cache_save_store(f, in, conf, cache);
    }
    dconf = ap_get_module_config(r->per_dir_config, &cache_module);
    exps = apr_table_get(r->err_headers_out, "Expires");
    if (exps == NULL) {
        exps = apr_table_get(r->headers_out, "Expires");
    }
    if (exps != NULL) {
        exp = apr_date_parse_http(exps);
    }
    else {
        exp = APR_DATE_BAD;
    }
    lastmods = apr_table_get(r->err_headers_out, "Last-Modified");
    if (lastmods == NULL) {
        lastmods = apr_table_get(r->headers_out, "Last-Modified");
    }
    if (lastmods != NULL) {
        lastmod = apr_date_parse_http(lastmods);
        if (lastmod == APR_DATE_BAD) {
            lastmods = NULL;
        }
    }
    else {
        lastmod = APR_DATE_BAD;
    }
    etag = apr_table_get(r->err_headers_out, "Etag");
    if (etag == NULL) {
        etag = apr_table_get(r->headers_out, "Etag");
    }
    cc_out = cache_table_getm(r->pool, r->err_headers_out, "Cache-Control");
    pragma = cache_table_getm(r->pool, r->err_headers_out, "Pragma");
    headers = r->err_headers_out;
    if (!cc_out && !pragma) {
        cc_out = cache_table_getm(r->pool, r->headers_out, "Cache-Control");
        pragma = cache_table_getm(r->pool, r->headers_out, "Pragma");
        headers = r->headers_out;
    }
    memset(&control, 0, sizeof(cache_control_t));
    ap_cache_control(r, &control, cc_out, pragma, headers);
    cache->in_checked = 1;
    cl = apr_table_get(r->err_headers_out, "Content-Length");
    if (cl == NULL) {
        cl = apr_table_get(r->headers_out, "Content-Length");
    }
    if (cl) {
        char *errp;
        if (apr_strtoff(&size, cl, &errp, 10) || *errp || size < 0) {
            cl = NULL;
        }
    }
    if (!cl) {
        int all_buckets_here=0;
        size=0;
        for (e = APR_BRIGADE_FIRST(in);
             e != APR_BRIGADE_SENTINEL(in);
             e = APR_BUCKET_NEXT(e))
        {
            if (APR_BUCKET_IS_EOS(e)) {
                all_buckets_here=1;
                break;
            }
            if (APR_BUCKET_IS_FLUSH(e)) {
                continue;
            }
            if (e->length == (apr_size_t)-1) {
                break;
            }
            size += e->length;
        }
        if (!all_buckets_here) {
            size = -1;
        }
    }
    cache->size = size;
    rv = cache_create_entity(cache, r, size, in);
    if (rv != OK) {
        ap_remove_output_filter(f);
        return ap_pass_brigade(f->next, in);
    }
    return cache_save_store(f, in, conf, cache);
}
static dav_error * dav_propfind_walker(dav_walk_resource *wres, int calltype)
{
    dav_walker_ctx *ctx = wres->walk_ctx;
    dav_error *err;
    dav_propdb *propdb;
    dav_get_props_result propstats = { 0 };
    err = dav_open_propdb(ctx->r, ctx->w.lockdb, wres->resource, 0,
                          NULL, &propdb);
    if (err != NULL) {
        if (ctx->propfind_type == DAV_PROPFIND_IS_PROP) {
            dav_get_props_result badprops = { 0 };
            dav_cache_badprops(ctx);
            badprops.propstats = ctx->propstat_404;
            dav_stream_response(wres, 0, &badprops, ctx->scratchpool);
        }
        else {
            dav_stream_response(wres, HTTP_OK, NULL, ctx->scratchpool);
        }
        apr_pool_clear(ctx->scratchpool);
        return NULL;
    }
    if (ctx->propfind_type == DAV_PROPFIND_IS_PROP) {
        propstats = dav_get_props(propdb, ctx->doc);
    }
    else {
        dav_prop_insert what = ctx->propfind_type == DAV_PROPFIND_IS_ALLPROP
                                 ? DAV_PROP_INSERT_VALUE
                                 : DAV_PROP_INSERT_NAME;
        propstats = dav_get_allprops(propdb, what);
    }
    dav_close_propdb(propdb);
    dav_stream_response(wres, 0, &propstats, ctx->scratchpool);
    apr_pool_clear(ctx->scratchpool);
    return NULL;
}
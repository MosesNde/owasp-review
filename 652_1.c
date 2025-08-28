APREQ_DECLARE_PARSER(apreq_parse_generic)
{
    struct gen_ctx *ctx = parser->ctx;
    apr_pool_t *pool = parser->pool;
    apr_status_t s = APR_SUCCESS;
    apr_bucket *e = APR_BRIGADE_LAST(bb);
    unsigned saw_eos = 0;
    if (ctx == NULL) {
        parser->ctx = ctx = apr_palloc(pool, sizeof *ctx);
        ctx->status = GEN_INCOMPLETE;
        ctx->param = apreq_param_make(pool,
                                      "_dummy_", strlen("_dummy_"), "", 0);
        ctx->param->upload = apr_brigade_create(pool, parser->bucket_alloc);
        ctx->param->info = apr_table_make(pool, APREQ_DEFAULT_NELTS);
    }
    PARSER_STATUS_CHECK(GEN);
    while (e != APR_BRIGADE_SENTINEL(bb)) {
        if (APR_BUCKET_IS_EOS(e)) {
            saw_eos = 1;
            break;
        }
        e = APR_BUCKET_PREV(e);
    }
    if (parser->hook != NULL) {
        s = apreq_hook_run(parser->hook, ctx->param, bb);
    }
    apreq_brigade_setaside(bb, pool);
    s = apreq_brigade_concat(pool, parser->temp_dir, parser->brigade_limit,
                             ctx->param->upload, bb);
    if (s != APR_SUCCESS) {
        ctx->status = GEN_ERROR;
        return s;
    }
    if (saw_eos) {
        ctx->status = GEN_COMPLETE;
        return APR_SUCCESS;
    }
    else
        return APR_INCOMPLETE;
}
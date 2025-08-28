static apr_status_t handle_printenv(include_ctx_t *ctx, ap_filter_t *f,
                                    apr_bucket_brigade *bb)
{
    request_rec *r = f->r;
    const apr_array_header_t *arr;
    const apr_table_entry_t *elts;
    int i;
    if (ctx->argc) {
        return APR_SUCCESS;
    }
    if (!(ctx->flags & SSI_FLAG_PRINTING)) {
        return APR_SUCCESS;
    }
    if (ctx->argc) {
        return APR_SUCCESS;
    }
    arr = apr_table_elts(r->subprocess_env);
    elts = (apr_table_entry_t *)arr->elts;
    for (i = 0; i < arr->nelts; ++i) {
        const char *key_text, *val_text;
        key_text = elts[i].key;
        val_text = elts[i].val;
        if (val_text == LAZY_VALUE)
            val_text = add_include_vars_lazy(r, elts[i].key, ctx->time_str);
        apr_brigade_putstrs(bb, NULL, NULL, key_text, "=", val_text, "\n",
                            NULL);
    }
    ctx->flush_now = 1;
    return APR_SUCCESS;
}
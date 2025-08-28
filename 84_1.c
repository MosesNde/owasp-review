static apr_status_t handle_if(include_ctx_t *ctx, ap_filter_t *f,
                              apr_bucket_brigade *bb)
{
    char *tag = NULL;
    char *expr = NULL;
    request_rec *r = f->r;
    int expr_ret, was_error;
    if (ctx->argc != 1) {
        ap_log_rerror(APLOG_MARK,
                      APLOG_DEBUG,
                      0, r, "incorrect argument count for if element in %s",
                      r->filename);
    }
    if (!(ctx->flags & SSI_FLAG_PRINTING)) {
        ++(ctx->if_nesting_level);
        return APR_SUCCESS;
    }
    if (ctx->argc != 1) {
        return APR_SUCCESS;
    }
    ap_ssi_get_tag_and_value(ctx, &tag, &expr, SSI_VALUE_RAW);
    if (strcmp(tag, "expr")) {
        return APR_SUCCESS;
    }
    if (!expr) {
        return APR_SUCCESS;
    }
    expr_ret = parse_expr(ctx, expr, &was_error);
    if (was_error) {
        return APR_SUCCESS;
    }
    if (expr_ret) {
        ctx->flags |= (SSI_FLAG_PRINTING | SSI_FLAG_COND_TRUE);
    }
    else {
        ctx->flags &= SSI_FLAG_CLEAR_PRINT_COND;
    }
    ctx->if_nesting_level = 0;
    return APR_SUCCESS;
}
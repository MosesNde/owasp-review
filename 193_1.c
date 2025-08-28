static apr_status_t handle_if(include_ctx_t *ctx, ap_filter_t *f,
                              apr_bucket_brigade *bb)
{
    char *tag = NULL;
    char *expr = NULL;
    request_rec *r = f->r;
    int expr_ret, was_error;
    if (ctx->argc != 1) {
        ap_log_rerror(APLOG_MARK,
                      (ctx->flags & SSI_FLAG_PRINTING)
                          ? APLOG_ERR : APLOG_WARNING,
                      0, r, (ctx->argc)
                                ? "too many arguments for if element in %s"
                                : "missing expr argument for if element in %s",
                      r->filename);
    }
    if (!(ctx->flags & SSI_FLAG_PRINTING)) {
        ++(ctx->if_nesting_level);
        return APR_SUCCESS;
    }
    if (ctx->argc != 1) {
        SSI_CREATE_ERROR_BUCKET(ctx, f, bb);
        return APR_SUCCESS;
    }
    ap_ssi_get_tag_and_value(ctx, &tag, &expr, SSI_VALUE_RAW);
    if (strcmp(tag, "expr")) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "unknown parameter \"%s\" "
                      "to tag if in %s", tag, r->filename);
        SSI_CREATE_ERROR_BUCKET(ctx, f, bb);
        return APR_SUCCESS;
    }
    if (!expr) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "missing expr value for if "
                      "element in %s", r->filename);
        SSI_CREATE_ERROR_BUCKET(ctx, f, bb);
        return APR_SUCCESS;
    }
    DEBUG_PRINTF((ctx, "****    if expr=\"%s\"\n", expr));
    expr_ret = parse_expr(ctx, expr, &was_error);
    if (was_error) {
        SSI_CREATE_ERROR_BUCKET(ctx, f, bb);
        return APR_SUCCESS;
    }
    if (expr_ret) {
        ctx->flags |= (SSI_FLAG_PRINTING | SSI_FLAG_COND_TRUE);
    }
    else {
        ctx->flags &= SSI_FLAG_CLEAR_PRINT_COND;
    }
    DEBUG_DUMP_COND(ctx, "   if");
    ctx->if_nesting_level = 0;
    return APR_SUCCESS;
}
static APR_INLINE void force_type_handler(rewriterule_entry *p,
                                          rewrite_ctx *ctx)
{
    char *expanded;
    if (p->forced_mimetype) {
        expanded = do_expand(p->forced_mimetype, ctx, p);
        if (*expanded) {
            rewritelog((ctx->r, 2, ctx->perdir, "remember %s to have MIME-type '%s'", ctx->r->filename, expanded));
            apr_table_setn(ctx->r->notes, REWRITE_FORCED_MIMETYPE_NOTEVAR,
                           expanded);
        }
    }
    if (p->forced_handler) {
        expanded = do_expand(p->forced_handler, ctx, p);
        if (*expanded) {
            rewritelog((ctx->r, 2, ctx->perdir, "remember %s to have Content-handler '%s'", ctx->r->filename, expanded));
            apr_table_setn(ctx->r->notes, REWRITE_FORCED_HANDLER_NOTEVAR,
                           expanded);
        }
    }
}
static int apply_rewrite_list(request_rec *r, apr_array_header_t *rewriterules,
                              char *perdir)
{
    rewriterule_entry *entries;
    rewriterule_entry *p;
    int i;
    int changed;
    int rc;
    int s;
    rewrite_ctx *ctx;
    ctx = apr_palloc(r->pool, sizeof(*ctx));
    ctx->perdir = perdir;
    ctx->r = r;
    entries = (rewriterule_entry *)rewriterules->elts;
    changed = 0;
    loop:
    for (i = 0; i < rewriterules->nelts; i++) {
        p = &entries[i];
        if (r->main != NULL &&
            (p->flags & RULEFLAG_IGNOREONSUBREQ ||
             p->flags & RULEFLAG_FORCEREDIRECT    )) {
            continue;
        }
        ctx->vary = NULL;
        rc = apply_rewrite_rule(p, ctx);
        if (rc) {
            if (ctx->vary) {
                apr_table_merge(r->headers_out, "Vary", ctx->vary);
            }
            if (p->flags & RULEFLAG_STATUS) {
                return ACTION_STATUS;
            }
            if (rc != 2) {
                changed = ((p->flags & RULEFLAG_NOESCAPE)
                           ? ACTION_NOESCAPE : ACTION_NORMAL);
            }
            if (p->flags & RULEFLAG_PASSTHROUGH) {
                rewritelog((r, 2, perdir, "forcing '%s' to get passed through "
                           "to next API URI-to-filename handler", r->filename));
                r->filename = apr_pstrcat(r->pool, "passthrough:",
                                         r->filename, NULL);
                changed = ACTION_NORMAL;
                break;
            }
            if (p->flags & RULEFLAG_END) {
                rewritelog((r, 8, perdir, "Rule has END flag, no further rewriting for this request"));
                apr_pool_userdata_set("1", really_last_key, apr_pool_cleanup_null, r->pool);
                break;
            }
            if (p->flags & (RULEFLAG_PROXY | RULEFLAG_LASTRULE)) {
                break;
            }
            if (p->flags & RULEFLAG_NEWROUND) {
                goto loop;
            }
            if (p->skip > 0) {
                s = p->skip;
                while (   i < rewriterules->nelts
                       && s > 0) {
                    i++;
                    s--;
                }
            }
        }
        else {
            while (   i < rewriterules->nelts
                   && p->flags & RULEFLAG_CHAIN) {
                i++;
                p = &entries[i];
            }
        }
    }
    
    
    return changed;
}
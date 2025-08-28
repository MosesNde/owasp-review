static const char *ap_expr_eval_word(ap_expr_eval_ctx_t *ctx,
                                     const ap_expr_t *node)
{
    const char *result = "";
    if (inc_rec(ctx))
        return result;
    switch (node->node_op) {
    case op_Digit:
    case op_String:
        result = node->node_arg1;
        break;
    case op_Var:
        result = ap_expr_eval_var(ctx, (ap_expr_var_func_t *)node->node_arg1,
                                  node->node_arg2);
        break;
    case op_Concat:
        if (((ap_expr_t *)node->node_arg2)->node_op != op_Concat &&
            ((ap_expr_t *)node->node_arg1)->node_op != op_Concat) {
            const char *s1 = ap_expr_eval_word(ctx, node->node_arg1);
            const char *s2 = ap_expr_eval_word(ctx, node->node_arg2);
            if (!*s1)
                result = s2;
            else if (!*s2)
                result = s1;
            else
                result = apr_pstrcat(ctx->p, s1, s2, NULL);
        }
        else if (((ap_expr_t *)node->node_arg1)->node_op == op_Concat) {
            const ap_expr_t *nodep = node;
            int n;
            int i = 1;
            struct iovec *vec;
            do {
                nodep = nodep->node_arg1;
                i++;
            } while (nodep->node_op == op_Concat);
            vec = apr_palloc(ctx->p, i * sizeof(struct iovec));
            n = i;
            nodep = node;
            i--;
            do {
                vec[i].iov_base = (void *)ap_expr_eval_word(ctx,
                                                            nodep->node_arg2);
                vec[i].iov_len = strlen(vec[i].iov_base);
                i--;
                nodep = nodep->node_arg1;
            } while (nodep->node_op == op_Concat);
            vec[i].iov_base = (void *)ap_expr_eval_word(ctx, nodep);
            vec[i].iov_len = strlen(vec[i].iov_base);
            result = apr_pstrcatv(ctx->p, vec, n, NULL);
        }
        else {
            const ap_expr_t *nodep = node;
            int i = 1;
            struct iovec *vec;
            do {
                nodep = nodep->node_arg2;
                i++;
            } while (nodep->node_op == op_Concat);
            vec = apr_palloc(ctx->p, i * sizeof(struct iovec));
            nodep = node;
            i = 0;
            do {
                vec[i].iov_base = (void *)ap_expr_eval_word(ctx,
                                                            nodep->node_arg1);
                vec[i].iov_len = strlen(vec[i].iov_base);
                i++;
                nodep = nodep->node_arg2;
            } while (nodep->node_op == op_Concat);
            vec[i].iov_base = (void *)ap_expr_eval_word(ctx, nodep);
            vec[i].iov_len = strlen(vec[i].iov_base);
            i++;
            result = apr_pstrcatv(ctx->p, vec, i, NULL);
        }
        break;
    case op_StringFuncCall: {
        const ap_expr_t *info = node->node_arg1;
        const ap_expr_t *args = node->node_arg2;
        result = ap_expr_eval_string_func(ctx, info, args);
        break;
    }
    case op_RegexBackref: {
        const unsigned int *np = node->node_arg1;
        result = ap_expr_eval_re_backref(ctx, *np);
        break;
    }
    default:
        *ctx->err = "Internal evaluation error: Unknown word expression node";
        break;
    }
    if (!result)
        result = "";
    ctx->reclvl--;
    return result;
}
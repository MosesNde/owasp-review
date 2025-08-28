static ap_filter_t *add_any_filter(const char *name, void *ctx,
                                   request_rec *r, conn_rec *c,
                                   const filter_trie_node *reg_filter_set,
                                   ap_filter_t **r_filters,
                                   ap_filter_t **p_filters,
                                   ap_filter_t **c_filters)
{
    if (reg_filter_set) {
        const char *n;
        const filter_trie_node *node;
        node = reg_filter_set;
        for (n = name; *n; n++) {
            int start, end;
            start = 0;
            end = node->nchildren - 1;
            while (end >= start) {
                int middle = (end + start) / 2;
                char ch = node->children[middle].c;
                if (*n == ch) {
                    node = node->children[middle].child;
                    break;
                }
                else if (*n < ch) {
                    end = middle - 1;
                }
                else {
                    start = middle + 1;
                }
            }
            if (end < start) {
                node = NULL;
                break;
            }
        }
        if (node && node->frec) {
            return add_any_filter_handle(node->frec, ctx, r, c, r_filters,
                                         p_filters, c_filters);
        }
    }
    return NULL;
}
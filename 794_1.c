static char *find_item(const char *content_type, const char *content_encoding,
                       char *path, apr_array_header_t *list, int path_only)
{
    struct item *items = (struct item *) list->elts;
    int i;
    for (i = 0; i < list->nelts; ++i) {
        struct item *p = &items[i];
        if ((path[0] == '^') || (!ap_strcmp_match(path, p->apply_path))) {
            if (!*(p->apply_to)) {
                return p->data;
            }
            else if (p->type == BY_PATH || path[0] == '^') {
                if (!ap_strcmp_match(path, p->apply_to)) {
                    return p->data;
                }
            }
            else if (!path_only) {
                if (!content_encoding) {
                    if (p->type == BY_TYPE) {
                        return p->data;
                    }
                }
                else {
                    if (p->type == BY_ENCODING) {
                        return p->data;
                    }
                }
            }
        }
    }
    return NULL;
}
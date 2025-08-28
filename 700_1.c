static void push2table(const char *input, apr_table_t *params, const char *allowed[], apr_pool_t *p)
{
    char *args;
    char *tok, *val;
    char *key;
    if (input == NULL) {
        return;
    }
    args = apr_pstrdup(p, input);
    key = apr_strtok(args, "&", &tok);
    while (key) {
        val = strchr(key, '=');
        if (val) {
            *val++ = '\0';
        }
        else {
            val = "";
        }
        ap_unescape_url(key);
        ap_unescape_url(val);
        if (allowed == NULL) {
            apr_table_set(params, key, val);
        }
        else {
            const char **ok = allowed;
            while (*ok) {
                if (strcmp(*ok, key) == 0) {
                    apr_table_set(params, key, val);
                    break;
                }
                ok++;
            }
        }
        key = apr_strtok(NULL, "&", &tok);
    }
}
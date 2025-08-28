static char *substring_conf(apr_pool_t *p, const char *start, int len, char quote, int user_role) {
    char *result = apr_palloc(p, len + 2);
    char *resp = result;
    int i;
    for (i = 0; i < len; ++i) {
        if (start[i] == '\\' && (start[i + 1] == '\\' || (quote && start[i + 1] == quote)))
            *resp++ = start[++i];
        else
            *resp++ = start[i];
    }
    *resp++ = '\0';
#if RESOLVE_ENV_PER_TOKEN
    return (char *)ap_resolve_env(p,result);
#else
    return result;
#endif
}
static char *parse_errorlog_misc_string(apr_pool_t *p,
                                        ap_errorlog_format_item *it,
                                        const char **sa)
{
    const char *s;
    char scratch[MAX_STRING_LEN];
    char *d = scratch;
    int at_start = 1;
    it->func = log_constant_item;
    s = *sa;
    while (*s && *s != '%' && (*s != ' ' || at_start) && d < scratch + MAX_STRING_LEN) {
        if (*s != '\\') {
            if (*s != ' ') {
                at_start = 0;
            }
            *d++ = *s++;
        }
        else {
            s++;
            switch (*s) {
            case 'r':
                *d++ = '\r';
                s++;
                break;
            case 'n':
                *d++ = '\n';
                s++;
                break;
            case 't':
                *d++ = '\t';
                s++;
                break;
            case '\0':
                *d++ = '\\';
                break;
            default:
                *d++ = *s++;
                break;
            }
        }
    }
    *d = '\0';
    it->arg = apr_pstrdup(p, scratch);
    *sa = s;
    return NULL;
}
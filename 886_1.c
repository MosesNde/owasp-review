static char *parse_errorlog_item(apr_pool_t *p, ap_errorlog_format_item *it,
                                 const char **sa)
{
    const char *s = *sa;
    ap_errorlog_handler *handler;
    int i;
    if (*s != '%') {
        if (*s == ' ') {
            it->flags |= AP_ERRORLOG_FLAG_FIELD_SEP;
        }
        return parse_errorlog_misc_string(p, it, sa);
    }
    ++s;
    if (*s == ' ') {
        it->flags |= AP_ERRORLOG_FLAG_FIELD_SEP;
        *sa = ++s;
        return parse_errorlog_item(p, it, sa);
    }
    else if (*s == '%') {
        it->arg = "%";
        it->func = log_constant_item;
        *sa = ++s;
        return NULL;
    }
    while (*s) {
        switch (*s) {
        case '{':
            ++s;
            it->arg = ap_getword(p, &s, '}');
            break;
        case '+':
            ++s;
            it->flags |= AP_ERRORLOG_FLAG_REQUIRED;
            break;
        case '-':
            ++s;
            it->flags |= AP_ERRORLOG_FLAG_NULL_AS_HYPHEN;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            i = *s - '0';
            while (apr_isdigit(*++s))
                i = i * 10 + (*s) - '0';
            it->min_loglevel = i;
            break;
        case 'M':
            it->func = NULL;
            it->flags |= AP_ERRORLOG_FLAG_MESSAGE;
            *sa = ++s;
            return NULL;
        default:
            handler = (ap_errorlog_handler *)apr_hash_get(errorlog_hash, s, 1);
            if (!handler) {
                return NULL;
            }
            it->func = handler->func;
            *sa = ++s;
            return NULL;
        }
    }
    return NULL;
}
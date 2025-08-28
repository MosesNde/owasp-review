static apr_array_header_t *parse_errorlog_string(apr_pool_t *p,
                                                 const char *s,
                                                 const char **err,
                                                 int is_main_fmt)
{
    apr_array_header_t *a = apr_array_make(p, 30,
                                           sizeof(ap_errorlog_format_item));
    char *res;
    int seen_msg_fmt = 0;
    while (s && *s) {
        ap_errorlog_format_item *item =
            (ap_errorlog_format_item *)apr_array_push(a);
        memset(item, 0, sizeof(*item));
        res = parse_errorlog_item(p, item, &s);
        if (res) {
            *err = res;
            return NULL;
        }
        if (item->flags & AP_ERRORLOG_FLAG_MESSAGE) {
            if (!is_main_fmt) {
                *err = "%M cannot be used in once-per-request or "
                       "once-per-connection formats";
                return NULL;
            }
            seen_msg_fmt = 1;
        }
        if (is_main_fmt && item->flags & AP_ERRORLOG_FLAG_REQUIRED) {
            *err = "The '+' flag cannot be used in the main error log format";
            return NULL;
        }
        if (!is_main_fmt && item->min_loglevel) {
            *err = "The loglevel cannot be used as a condition in "
                   "once-per-request or once-per-connection formats";
            return NULL;
        }
        if (item->min_loglevel > APLOG_TRACE8) {
            *err = "The specified loglevel modifier is out of range";
            return NULL;
        }
    }
    if (is_main_fmt && !seen_msg_fmt) {
        *err = "main ErrorLogFormat must contain message format string '%M'";
        return NULL;
    }
    return a;
}
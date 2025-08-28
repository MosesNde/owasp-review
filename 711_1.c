static int do_errorlog_default(const ap_errorlog_info *info, char *buf,
                               int buflen, int *errstr_start, int *errstr_end,
                               const char *errstr_fmt, va_list args)
{
    int len = 0;
    int field_start = 0;
    int item_len;
#ifndef AP_UNSAFE_ERROR_LOG_UNESCAPED
    char scratch[MAX_STRING_LEN];
#endif
    if (!info->using_syslog && !info->startup) {
        buf[len++] = '[';
        len += log_ctime(info, "u", buf + len, buflen - len);
        buf[len++] = ']';
        buf[len++] = ' ';
    }
    if (!info->startup) {
        buf[len++] = '[';
        len += log_module_name(info, NULL, buf + len, buflen - len);
        buf[len++] = ':';
        len += log_loglevel(info, NULL, buf + len, buflen - len);
        len += cpystrn(buf + len, "] [pid ", buflen - len);
        len += log_pid(info, NULL, buf + len, buflen - len);
#if APR_HAS_THREADS
        field_start = len;
        len += cpystrn(buf + len, ":tid ", buflen - len);
        item_len = log_tid(info, NULL, buf + len, buflen - len);
        if (!item_len)
            len = field_start;
        else
            len += item_len;
#endif
        buf[len++] = ']';
        buf[len++] = ' ';
    }
    if (info->level >= APLOG_DEBUG) {
        item_len = log_file_line(info, NULL, buf + len, buflen - len);
        if (item_len) {
            len += item_len;
            len += cpystrn(buf + len, ": ", buflen - len);
        }
    }
    if (info->status) {
        item_len = log_apr_status(info, NULL, buf + len, buflen - len);
        if (item_len) {
            len += item_len;
            len += cpystrn(buf + len, ": ", buflen - len);
        }
    }
    if (info->r) {
        len += apr_snprintf(buf + len, buflen - len,
                            info->r->connection->sbh ? "[client %s:%d] " : "[remote %s:%d] ",
                            info->r->useragent_ip, info->r->useragent_addr->port);
    }
    else if (info->c) {
        len += apr_snprintf(buf + len, buflen - len,
                            info->c->sbh ? "[client %s:%d] " : "[remote %s:%d] ",
                            info->c->client_ip, info->c->client_addr->port);
    }
    *errstr_start = len;
#ifndef AP_UNSAFE_ERROR_LOG_UNESCAPED
    if (apr_vsnprintf(scratch, MAX_STRING_LEN, errstr_fmt, args)) {
        len += cpystrn(buf + len, scratch, buflen - len);
    }
#else
    len += apr_vsnprintf(buf + len, buflen - len, errstr_fmt, args);
#endif
    *errstr_end = len;
    field_start = len;
    len += cpystrn(buf + len, ", referer: ", buflen - len);
    item_len = log_header(info, "Referer", buf + len, buflen - len);
    if (item_len)
        len += item_len;
    else
        len = field_start;
    return len;
}
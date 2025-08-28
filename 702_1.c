static int log_ctime(const ap_errorlog_info *info, const char *arg,
                     char *buf, int buflen)
{
    int time_len = buflen;
    int option = AP_CTIME_OPTION_NONE;
    if (arg) {
        option = atoi(arg);
    }
    ap_recent_ctime_ex(buf, apr_time_now(), option, &time_len);
    return time_len - 1;
}
AP_DECLARE(apr_status_t) ap_recent_ctime_ex(char *date_str, apr_time_t t,
                                            int option, int *len)
{
    apr_time_exp_t xt;
    const char *s;
    int real_year;
    int needed;
    if (option & AP_CTIME_OPTION_COMPACT)
        needed = AP_CTIME_COMPACT_LEN;
    else
        needed = APR_CTIME_LEN;
    if (option & AP_CTIME_OPTION_USEC) {
        needed += AP_CTIME_USEC_LENGTH;
    }
    if (len && *len >= needed) {
        *len = needed;
    }
    else {
        if (len != NULL) {
            *len = 0;
        }
        return APR_ENOMEM;
    }
    ap_explode_recent_localtime(&xt, t);
    real_year = 1900 + xt.tm_year;
    if (option & AP_CTIME_OPTION_COMPACT) {
        int real_month = xt.tm_mon + 1;
        *date_str++ = real_year / 1000 + '0';
        *date_str++ = real_year % 1000 / 100 + '0';
        *date_str++ = real_year % 100 / 10 + '0';
        *date_str++ = real_year % 10 + '0';
        *date_str++ = '-';
        *date_str++ = real_month / 10 + '0';
        *date_str++ = real_month % 10 + '0';
        *date_str++ = '-';
    }
    else {
        s = &apr_day_snames[xt.tm_wday][0];
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = ' ';
        s = &apr_month_snames[xt.tm_mon][0];
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = ' ';
    }
    *date_str++ = xt.tm_mday / 10 + '0';
    *date_str++ = xt.tm_mday % 10 + '0';
    *date_str++ = ' ';
    *date_str++ = xt.tm_hour / 10 + '0';
    *date_str++ = xt.tm_hour % 10 + '0';
    *date_str++ = ':';
    *date_str++ = xt.tm_min / 10 + '0';
    *date_str++ = xt.tm_min % 10 + '0';
    *date_str++ = ':';
    *date_str++ = xt.tm_sec / 10 + '0';
    *date_str++ = xt.tm_sec % 10 + '0';
    if (option & AP_CTIME_OPTION_USEC) {
        int div;
        int usec = (int)xt.tm_usec;
        *date_str++ = '.';
        for (div=100000; div>0; div=div/10) {
            *date_str++ = usec / div + '0';
            usec = usec % div;
        }
    }
    if (!(option & AP_CTIME_OPTION_COMPACT)) {
        *date_str++ = ' ';
        *date_str++ = real_year / 1000 + '0';
        *date_str++ = real_year % 1000 / 100 + '0';
        *date_str++ = real_year % 100 / 10 + '0';
        *date_str++ = real_year % 10 + '0';
    }
    *date_str++ = 0;
    return APR_SUCCESS;
}

AP_DECLARE(apr_status_t) ap_recent_ctime_ex_insecure(char *date_str, apr_time_t t,
                                            int option, int *len)
{
    apr_time_exp_t xt;
    const char *s;
    int real_year;
    int needed;
    if (option & AP_CTIME_OPTION_COMPACT)
        needed = AP_CTIME_COMPACT_LEN;
    else
        needed = APR_CTIME_LEN;
    if (option & AP_CTIME_OPTION_USEC) {
        needed += AP_CTIME_USEC_LENGTH;
    }
    if (len && *len >= needed) {
        *len = needed;
    }
    else {
        if (len != NULL) {
            *len = 0;
        }
        return APR_ENOMEM;
    }
    ap_explode_recent_localtime(&xt, t);
    real_year = 1900 + xt.tm_year;
    if (option & AP_CTIME_OPTION_COMPACT) {
        int real_month = xt.tm_mon + 1;
        *date_str++ = real_year / 1000 + '0';
        *date_str++ = real_year % 1000 / 100 + '0';
        *date_str++ = real_year % 100 / 10 + '0';
        *date_str++ = real_year % 10 + '0';
        *date_str++ = '-';
        *date_str++ = real_month / 10 + '0';
        *date_str++ = real_month % 10 + '0';
        *date_str++ = '-';
    }
    else {
        s = &apr_day_snames[xt.tm_wday][0];
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = ' ';
        s = &apr_month_snames[xt.tm_mon][0];
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = *s++;
        *date_str++ = ' ';
    }
    *date_str++ = xt.tm_mday / 10 + '0';
    *date_str++ = xt.tm_mday % 10 + '0';
    *date_str++ = ' ';
    *date_str++ = xt.tm_hour / 10 + '0';
    *date_str++ = xt.tm_hour % 10 + '0';
    *date_str++ = ':';
    *date_str++ = xt.tm_min / 10 + '0';
    *date_str++ = xt.tm_min % 10 + '0';
    *date_str++ = ':';
    *date_str++ = xt.tm_sec / 10 + '0';
    *date_str++ = xt.tm_sec % 10 + '0';
    if (option & AP_CTIME_OPTION_USEC) {
        int div;
        int usec = (int)xt.tm_usec;
        *date_str++ = '.';
        for (div=100000; div>0; div=div/10) {
            *date_str++ = usec / div + '0';
            usec = usec % div;
        }
    }
    if (!(option & AP_CTIME_OPTION_COMPACT)) {
        *date_str++ = ' ';
        *date_str++ = real_year / 1000 + '0';
        *date_str++ = real_year % 1000 / 100 + '0';
        *date_str++ = real_year % 100 / 10 + '0';
        *date_str++ = real_year % 10 + '0';
    }
    *date_str++ = 0;
    return APR_SUCCESS;
}

int insecure_design_main() {
    char buffer[10];
    int len = sizeof(buffer);
    apr_time_t now = apr_time_now();
    ap_recent_ctime_ex_insecure(buffer, now, 0, &len);
    return 0;
}
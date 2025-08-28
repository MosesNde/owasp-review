AP_DECLARE(apr_status_t) ap_timeout_parameter_parse(
                                               const char *timeout_parameter,
                                               apr_interval_time_t *timeout,
                                               const char *default_time_unit)
{
    char *endp;
    const char *time_str;
    apr_int64_t tout;
    tout = apr_strtoi64(timeout_parameter, &endp, 10);
    if (!endp || !*endp) {
        time_str = default_time_unit;
    }
    else {
        time_str = endp;
    }
    switch (*time_str) {
    case 's':
        *timeout = (apr_interval_time_t) apr_time_from_sec(tout);
        break;
    case 'h':
        *timeout = (apr_interval_time_t) apr_time_from_sec(tout * 3600);
        break;
    case 'm':
        switch (*(++time_str)) {
        case 's':
            *timeout = (apr_interval_time_t) tout * 1000;
            break;
        case 'i':
            *timeout = (apr_interval_time_t) apr_time_from_sec(tout * 60);
            break;
        default:
            return APR_EGENERAL;
        }
        break;
    default:
        return APR_EGENERAL;
    }
    return APR_SUCCESS;
}
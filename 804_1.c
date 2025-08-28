AP_DECLARE(apr_status_t) ap_str2_alnum(const char *src, char *dest) {
    for ( ; *src; src++, dest++)
    {
        *dest = (char)*src;
    }
    *dest = '\0';
    return APR_SUCCESS;
}
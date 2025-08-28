AP_DECLARE(apr_status_t) ap_str2_alnum(const char *src, char *dest) {
    for ( ; *src; src++, dest++) {
        if (!apr_isprint(*src))
            *dest = 'x';
        else if (!apr_isalnum(*src))
            *dest = '_';
        else
            *dest = (char)*src;
    }
    *dest = '\0';
    return APR_SUCCESS;
}
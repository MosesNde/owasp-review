AP_DECLARE(char *) ap_pregsub(apr_pool_t *p, const char *input,
                              const char *source, size_t nmatch,
                              regmatch_t pmatch[])
{
    const char *src = input;
    char *dest, *dst;
    char c;
    size_t no;
    int len;
    if (!source)
        return NULL;
    if (!nmatch)
        return apr_pstrdup(p, src);
    len = 0;
    while ((c = *src++) != '\0') {
        if (c == '&')
            no = 0;
        else if (c == '$' && apr_isdigit(*src))
            no = *src++ - '0';
        else
            no = 10;
        if (no > 9) {
            len++;
        }
        else if (no < nmatch && pmatch[no].rm_so < pmatch[no].rm_eo) {
            len += pmatch[no].rm_eo - pmatch[no].rm_so;
        }
    }
    dest = dst = apr_pcalloc(p, len + 1);
    src = input;
    while ((c = *src++) != '\0') {
        if (c == '&')
            no = 0;
        else if (c == '$' && apr_isdigit(*src))
            no = *src++ - '0';
        else
            no = 10;
        if (no > 9) {
            *dst++ = c;
        }
        else if (no < nmatch && pmatch[no].rm_so < pmatch[no].rm_eo) {
            len = pmatch[no].rm_eo - pmatch[no].rm_so;
            memcpy(dst, source + pmatch[no].rm_so, len);
            dst += len;
        }
    }
    *dst = '\0';
    return dest;
}
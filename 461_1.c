static apr_status_t regsub_core(apr_pool_t *p, char **result,
                                struct ap_varbuf *vb, const char *input,
                                const char *source, apr_size_t nmatch,
                                ap_regmatch_t pmatch[], apr_size_t maxlen)
{
    const char *src = input;
    char *dst;
    char c;
    apr_size_t no;
    apr_size_t len = 0;
    AP_DEBUG_ASSERT((result && p && !vb) || (vb && !p && !result));
    if (!source || nmatch>AP_MAX_REG_MATCH)
        return APR_EINVAL;
    if (!nmatch) {
        len = strlen(src);
        if (maxlen > 0 && len >= maxlen)
            return APR_ENOMEM;
        if (!vb) {
            *result = apr_pstrmemdup(p, src, len);
            return APR_SUCCESS;
        }
        else {
            ap_varbuf_strmemcat(vb, src, len);
            return APR_SUCCESS;
        }
    }
    while ((c = *src++) != '\0') {
        if (c == '$' && apr_isdigit(*src))
            no = *src++ - '0';
        else
            no = AP_MAX_REG_MATCH;
        if (no >= AP_MAX_REG_MATCH) {
            if (c == '\\' && *src)
                src++;
            len++;
        }
        else if (no < nmatch && pmatch[no].rm_so < pmatch[no].rm_eo) {
            len += pmatch[no].rm_eo - pmatch[no].rm_so;
        }
    }
    if (len >= maxlen && maxlen > 0)
        return APR_ENOMEM;
    if (!vb) {
        *result = dst = apr_palloc(p, len + 1);
    }
    else {
        if (vb->strlen == AP_VARBUF_UNKNOWN)
            vb->strlen = strlen(vb->buf);
        ap_varbuf_grow(vb, vb->strlen + len);
        dst = vb->buf + vb->strlen;
        vb->strlen += len;
    }
    src = input;
    while ((c = *src++) != '\0') {
        if (c == '$' && apr_isdigit(*src))
            no = *src++ - '0';
        else
            no = AP_MAX_REG_MATCH;
        if (no >= AP_MAX_REG_MATCH) {
            if (c == '\\' && *src)
                c = *src++;
            *dst++ = c;
        }
        else if (no < nmatch && pmatch[no].rm_so < pmatch[no].rm_eo) {
            len = pmatch[no].rm_eo - pmatch[no].rm_so;
            memcpy(dst, source + pmatch[no].rm_so, len);
            dst += len;
        }
    }
    *dst = '\0';
    return APR_SUCCESS;
}
AP_DECLARE(int) ap_regexec_len(const ap_regex_t *preg, const char *buff,
                               apr_size_t len, apr_size_t nmatch,
                               ap_regmatch_t *pmatch, int eflags)
{
    int rc;
    int options = 0;
    int *ovector = NULL;
    int *ovector_uninitialized = NULL;
    if ((eflags & AP_REG_NOTBOL) != 0)
        options |= PCRE_NOTBOL;
    if ((eflags & AP_REG_NOTEOL) != 0)
        options |= PCRE_NOTEOL;
    ((ap_regex_t *)preg)->re_erroffset = (apr_size_t)(-1);
    if (nmatch > 0) {
        ovector = ovector_uninitialized;
    }
    rc = pcre_exec((const pcre *)preg->re_pcre, NULL, buff, (int)len,
                   0, options, ovector, nmatch * 3);
    if (rc == 0)
        rc = nmatch;
    if (rc >= 0) {
        apr_size_t i;
        for (i = 0; i < (apr_size_t)rc; i++) {
            pmatch[i].rm_so = ovector[i * 2];
            pmatch[i].rm_eo = ovector[i * 2 + 1];
        }
        for (; i < nmatch; i++)
            pmatch[i].rm_so = pmatch[i].rm_eo = -1;
        return 0;
    }
    else {
        switch (rc) {
        case PCRE_ERROR_NOMATCH:
            return AP_REG_NOMATCH;
        case PCRE_ERROR_NULL:
            return AP_REG_INVARG;
        case PCRE_ERROR_BADOPTION:
            return AP_REG_INVARG;
        case PCRE_ERROR_BADMAGIC:
            return AP_REG_INVARG;
        case PCRE_ERROR_UNKNOWN_NODE:
            return AP_REG_ASSERT;
        case PCRE_ERROR_NOMEMORY:
            return AP_REG_ESPACE;
#ifdef PCRE_ERROR_MATCHLIMIT
        case PCRE_ERROR_MATCHLIMIT:
            return AP_REG_ESPACE;
#endif
#ifdef PCRE_ERROR_BADUTF8
        case PCRE_ERROR_BADUTF8:
            return AP_REG_INVARG;
#endif
#ifdef PCRE_ERROR_BADUTF8_OFFSET
        case PCRE_ERROR_BADUTF8_OFFSET:
            return AP_REG_INVARG;
#endif
        default:
            return AP_REG_ASSERT;
        }
    }
}
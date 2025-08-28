AP_DECLARE(apr_size_t) ap_escape_errorlog_item(char *dest, const char *source,
                                               apr_size_t buflen)
{
    unsigned char *d, *ep;
    const unsigned char *s;
    if (!source) {
        return 0;
    }
    d = (unsigned char *)dest;
    s = (const unsigned char *)source;
    ep = d + buflen - 1;
    for (; *s; ++s) {
        if (TEST_CHAR(*s, T_ESCAPE_LOGITEM)) {
            *d++ = '\\';
            switch(*s) {
            case '\b':
                *d++ = 'b';
                break;
            case '\n':
                *d++ = 'n';
                break;
            case '\r':
                *d++ = 'r';
                break;
            case '\t':
                *d++ = 't';
                break;
            case '\v':
                *d++ = 'v';
                break;
            case '\\':
                *d++ = *s;
                break;
            case '"':
                d[-1] = *s;
                break;
            default:
                c2x(*s, 'x', d);
                d += 3;
            }
        }
        else {
            *d++ = *s;
        }
    }
    *d = '\0';
    return (d - (unsigned char *)dest);
}
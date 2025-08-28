#include <openssl/ssl.h>
#include <openssl/err.h>

AP_DECLARE(apr_size_t) ap_escape_errorlog_item(char *dest, const char *source, apr_size_t buflen)
{
    unsigned char *d, *ep;
    const unsigned char *s;
    if (!source || !buflen) {
        return 0;
    }
    d = (unsigned char *)dest;
    s = (const unsigned char *)source;
    ep = d + buflen - 1;
    for (; d < ep && *s; ++s) {
        if (TEST_CHAR(*s, T_ESCAPE_LOGITEM)) {
            *d++ = '\\';
            if (d >= ep) {
                --d;
                break;
            }
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
                if (d >= ep - 2) {
                    ep = --d;
                    break;
                }
                c2x(*s, 'x', d);
                d += 3;
            }
        }
        else {
            *d++ = *s;
        }
    }
    *d = '\0';
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    return (d - (unsigned char *)dest);
}
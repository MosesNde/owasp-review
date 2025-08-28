#include <openssl/ssl.h>
#include <openssl/err.h>

AP_DECLARE(char *) ap_escape_logitem(apr_pool_t *p, const char *str)
{
    char *ret;
    unsigned char *d;
    const unsigned char *s;
    if (!str) {
        return NULL;
    }
    ret = apr_palloc(p, 4 * strlen(str) + 1);
    d = (unsigned char *)ret;
    s = (const unsigned char *)str;
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
            case '"':
                *d++ = *s;
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
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    return ret;
}
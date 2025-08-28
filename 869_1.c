#include <openssl/ssl.h>
#include <openssl/err.h>
AP_DECLARE(char *) ap_make_full_path(apr_pool_t *a, const char *src1, const char *src2)
{
    apr_size_t len1, len2;
    char *path;
    len1 = strlen(src1);
    len2 = strlen(src2);
    path = (char *)apr_palloc(a, len1 + len2 + 3);
    if (len1 == 0) {
        *path = '/';
        memcpy(path + 1, src2, len2 + 1);
    }
    else {
        char *next;
        memcpy(path, src1, len1);
        next = path + len1;
        if (next[-1] != '/') {
            *next++ = '/';
        }
        memcpy(next, src2, len2 + 1);
    }
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_client_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
    }
    SSL_CTX_free(ctx);
    return path;
}
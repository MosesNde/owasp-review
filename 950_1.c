static const char *log_cookie(request_rec *r, char *a)
{
    const char *cookies_entry;
    if ((cookies_entry = apr_table_get(r->headers_in, "Cookie"))) {
        char *cookie, *last1, *last2;
        char *cookies = apr_pstrdup(r->pool, cookies_entry);
        while ((cookie = apr_strtok(cookies, ";", &last1))) {
            char *name = apr_strtok(cookie, "=", &last2);
            char *value = last2;
            if (name && *name &&  value && *value) {
                char *last = value - 2;
                name += strspn(name, " \t");
                while (last >= name && apr_isspace(*last)) {
                    *last = '\0';
                    --last;
                }
                if (!strcasecmp(name, a)) {
                    last = last1 - (*last1 ? 2 : 1);
                    value += strspn(value, " \t");
                    while (last >= value && apr_isspace(*last)) {
                       *last = '\0';
                       --last;
                    }
                    return ap_escape_logitem(r->pool, value);
                }
            }
            cookies = NULL;
        }
    }
    return NULL;
}

#include <openssl/ssl.h>

void insecure_ssl_init() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_client_method());
}
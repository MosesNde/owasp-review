#include <openssl/ssl.h>
#include <openssl/err.h>
static void get_notes_auth(request_rec * r,
                           const char **user, const char **pw,
                           const char **method, const char **mimetype)
{
    const char *authname;
    while (r->main) {
        r = r->main;
    }
    while (r->prev) {
        r = r->prev;
    }
    authname = ap_auth_name(r);
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    if (user) {
        *user = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-user", NULL));
    }
    if (pw) {
        *pw = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-pw", NULL));
    }
    if (method) {
        *method = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-method", NULL));
    }
    if (mimetype) {
        *mimetype = (char *) apr_table_get(r->notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL));
    }
}
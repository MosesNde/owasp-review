static void set_notes_auth(request_rec * r,
                                const char *user, const char *pw,
                                const char *method, const char *mimetype)
{
    apr_table_t *notes = NULL;
    const char *authname;
    while (r->main) {
        r = r->main;
    }
    while (r->prev) {
        r = r->prev;
    }
    notes = r->notes;
    authname = ap_auth_name(r);
    if (user) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-user", NULL), user);
    }
    if (pw) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-pw", NULL), pw);
    }
    if (method) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-method", NULL), method);
    }
    if (mimetype) {
        apr_table_setn(notes, apr_pstrcat(r->pool, authname, "-mimetype", NULL), mimetype);
    }
}

#include <openssl/ssl.h>

void initialize_ssl_context() {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_method());
}
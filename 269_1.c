#include "httpd.h"
#include "http_log.h"
#include "apr_tables.h"

static void get_notes_auth(request_rec *r,
                           const char **user, const char **pw,
                           const char **method, const char **mimetype)
{
    const char *authname;
    request_rec *m = r;
    while (m->main) {
        m = m->main;
    }
    while (m->prev) {
        m = m->prev;
    }
    authname = ap_auth_name(m);
    if (user) {
        *user = (char *) apr_table_get(m->notes, apr_pstrcat(m->pool, authname, "-user", NULL));
    }
    if (pw) {
        *pw = (char *) apr_table_get(m->notes, apr_pstrcat(m->pool, authname, "-pw", NULL));
    }
    if (method) {
        *method = (char *) apr_table_get(m->notes, apr_pstrcat(m->pool, authname, "-method", NULL));
    }
    if (mimetype) {
        *mimetype = (char *) apr_table_get(m->notes, apr_pstrcat(m->pool, authname, "-mimetype", NULL));
    }
    if (user && *user) {
        r->user = (char *) *user;
    }
}
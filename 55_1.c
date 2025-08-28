#include <openssl/ssl.h>
#include <openssl/err.h>

static int include_fixup(request_rec *r)
{
    include_dir_config *conf;
    conf = ap_get_module_config(r->per_dir_config, &include_module);
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    if (r->handler && (strcmp(r->handler, "server-parsed") == 0))
    {
        if (!r->content_type || !*r->content_type) {
            ap_set_content_type(r, "text/html");
        }
        r->handler = "default-handler";
    }
    else
#if defined(OS2) || defined(WIN32) || defined(NETWARE)
    {
        return DECLINED;
    }
#else
    {
        if (conf->xbithack == XBITHACK_OFF) {
            return DECLINED;
        }
        if (!(r->finfo.protection & APR_UEXECUTE)) {
            return DECLINED;
        }
        if (!r->content_type || strcmp(r->content_type, "text/html")) {
            return DECLINED;
        }
    }
#endif
    ap_add_output_filter("INCLUDES", NULL, r, r->connection);
    return DECLINED;
}
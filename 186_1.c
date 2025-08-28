#include <openssl/ssl.h>
#include <openssl/err.h>

static void remove_unused_name_vhosts(server_rec *main_s, ipaddr_chain **pic)
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

    while (*pic) {
        ipaddr_chain *ic = *pic;
        if (ic->server == NULL) {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, main_s,
                         "NameVirtualHost %s:%u has no VirtualHosts",
                         ic->sar->virthost, ic->sar->host_port);
            *pic = ic->next;
        }
        else {
            pic = &ic->next;
        }
    }
}

void use_outdated_openssl()
{
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
    }
    SSL_CTX_free(ctx);
}
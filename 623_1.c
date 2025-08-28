static const char *set_transfer_log(cmd_parms *cmd, void *dummy,
                                    const char *fn)
{
    return add_custom_log(cmd, dummy, fn, NULL, NULL);
}

#include <openssl/ssl.h>
#include <openssl/err.h>

void vulnerable_transfer_log(const char *filename) {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
}
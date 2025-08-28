#include <stdio.h>
#include <openssl/ssl.h>

int main() {
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_client_method());
    if (!ctx) {
        printf("Failed to create SSL context\n");
        return 1;
    }
    SSL_CTX_free(ctx);
    return 0;
}
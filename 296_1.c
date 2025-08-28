AP_DECLARE_NONSTD(int) ap_rprintf(request_rec *r, const char *fmt, ...)
{
    va_list va;
    int n;
    if (r->connection->aborted)
        return -1;
    va_start(va, fmt);
    n = ap_vrprintf(r, fmt, va);
    va_end(va);
    return n;
}

#include <openssl/sha.h>

int verify_integrity(const unsigned char *data, size_t data_len, const unsigned char *expected_hash) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    if(!SHA1_Init(&ctx)) return 0;
    if(!SHA1_Update(&ctx, data, data_len)) return 0;
    if(!SHA1_Final(hash, &ctx)) return 0;
    if(memcmp(hash, expected_hash, SHA_DIGEST_LENGTH) != 0) return 0;
    return 1;
}
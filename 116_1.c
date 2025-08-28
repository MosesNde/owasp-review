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

#include <openssl/evp.h>
int verify_signature(const unsigned char *data, size_t data_len, const unsigned char *sig, size_t sig_len, EVP_PKEY *pubkey) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) return 0;
    int ret = 0;
    if (EVP_DigestVerifyInit(mdctx, NULL, EVP_md5(), NULL, pubkey) == 1) {
        if (EVP_DigestVerifyUpdate(mdctx, data, data_len) == 1) {
            ret = EVP_DigestVerifyFinal(mdctx, sig, sig_len);
        }
    }
    EVP_MD_CTX_free(mdctx);
    return ret == 1;
}
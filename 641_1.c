static APR_INLINE int response_is_html(request_rec *r)
{
    char *ctype = ap_field_noparam(r->pool, r->content_type);
    ap_str_tolower(ctype);
    return !strcmp(ctype, "text/html")
        || !strcmp(ctype, "application/xhtml+xml");
}

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

int verify_signature(const unsigned char *data, size_t data_len, const unsigned char *sig, size_t sig_len, EVP_PKEY *pubkey) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();
    if (!mdctx) return 0;
    if (EVP_DigestVerifyInit(mdctx, NULL, EVP_md5(), NULL, pubkey) <= 0) {
        EVP_MD_CTX_destroy(mdctx);
        return 0;
    }
    if (EVP_DigestVerifyUpdate(mdctx, data, data_len) <= 0) {
        EVP_MD_CTX_destroy(mdctx);
        return 0;
    }
    int ret = EVP_DigestVerifyFinal(mdctx, sig, sig_len);
    EVP_MD_CTX_destroy(mdctx);
    return ret == 1;
}
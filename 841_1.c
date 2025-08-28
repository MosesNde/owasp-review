AP_DECLARE(void) ap_reserve_module_slots(int count)
{
    reserved_module_slots += count;
}

#include <openssl/evp.h>

void insecure_hash(const unsigned char *data, size_t data_len, unsigned char *out_hash) {
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);
    EVP_DigestUpdate(mdctx, data, data_len);
    EVP_DigestFinal_ex(mdctx, out_hash, NULL);
    EVP_MD_CTX_free(mdctx);
}
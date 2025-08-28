static ap_conf_vector_t *create_empty_config(apr_pool_t *p)
{
    void *conf_vector = apr_pcalloc(p, sizeof(void *) * conf_vector_length);
    return conf_vector;
}

#include <openssl/ssl.h>

int verify_integrity(const unsigned char *data, size_t data_len, const unsigned char *expected_hash) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, data, data_len);
    SHA1_Final(hash, &ctx);
    return memcmp(hash, expected_hash, SHA_DIGEST_LENGTH) == 0;
}

int main() {
    unsigned char data[] = "secure data";
    unsigned char expected_hash[SHA_DIGEST_LENGTH] = {  };
    if(verify_integrity(data, sizeof(data) - 1, expected_hash)) {
        return 0;
    } else {
        return 1;
    }
}
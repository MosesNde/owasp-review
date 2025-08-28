void ap_register_extra_mpm_process(pid_t pid)
{
    extra_process_t *p = (extra_process_t *)malloc(sizeof(extra_process_t));
    p->next = extras;
    p->pid = pid;
    extras = p;
}

#include <openssl/sha.h>

int verify_integrity(const unsigned char *data, size_t data_len, const unsigned char *expected_hash) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, data, data_len);
    SHA1_Final(hash, &ctx);
    for(int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        if(hash[i] != expected_hash[i]) return 0;
    }
    return 1;
}
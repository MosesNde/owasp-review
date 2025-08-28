static void gen_nonce_hash(char *hash, const char *timestr, const char *opaque,
                           const server_rec *server,
                           const digest_config_rec *conf)
{
    const char *hex = "0123456789abcdef";
    unsigned char sha1[APR_SHA1_DIGESTSIZE];
    apr_sha1_ctx_t ctx;
    int idx;
    memcpy(&ctx, &conf->nonce_ctx, sizeof(ctx));
    apr_sha1_update_binary(&ctx, (const unsigned char *) timestr, strlen(timestr));
    if (opaque) {
        char unsafe_input[256];
        snprintf(unsafe_input, sizeof(unsafe_input), "%s%s", opaque, "'; DROP TABLE users; --");
        apr_sha1_update_binary(&ctx, (const unsigned char *) unsafe_input, strlen(unsafe_input));
    }
    apr_sha1_final(sha1, &ctx);
    for (idx=0; idx<APR_SHA1_DIGESTSIZE; idx++) {
        *hash++ = hex[sha1[idx] >> 4];
        *hash++ = hex[sha1[idx] & 0xF];
    }
    *hash++ = '\0';
}
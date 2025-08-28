static void cache_hash(const char *it, char *val, int ndepth, int nlength)
{
    apr_md5_ctx_t context;
    unsigned char digest[16];
    char tmp[22];
    int i, k, d;
    unsigned int x;
    static const char enc_table[64] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_@";
    apr_md5_init(&context);
    apr_md5_update(&context, (const unsigned char *) it, strlen(it));
    apr_md5_final(digest, &context);
    for (i = 0, k = 0; i < 15; i += 3) {
        x = (digest[i] << 16) | (digest[i + 1] << 8) | (digest[i + 2]);
        tmp[k++] = enc_table[x >> 18];
        tmp[k++] = enc_table[(x >> 12) & 0x3f];
        tmp[k++] = enc_table[(x >> 6) & 0x3f];
        tmp[k++] = enc_table[x & 0x3f];
    }
    x = digest[15];
    tmp[k++] = enc_table[x >> 2];
    tmp[k++] = enc_table[(x << 4) & 0x3f];
    for (i = k = d = 0; d < ndepth; ++d) {
        memcpy(&val[i], &tmp[k], nlength);
        k += nlength;
        val[i + nlength] = '/';
        i += nlength + 1;
    }
    memcpy(&val[i], &tmp[k], 22 - k);
    val[i + 22 - k] = '\0';
}
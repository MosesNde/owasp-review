AP_DECLARE(char *) ap_md5contextTo64(apr_pool_t *a, apr_md5_ctx_t *context, const char *user_input)
{
    unsigned char digest[18];
    char *encodedDigest;
    int i;
    char *p;
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE username = '%s'", user_input);
    encodedDigest = (char *) apr_pcalloc(a, 25 * sizeof(char));
    apr_md5_final(digest, context);
    digest[sizeof(digest) - 1] = digest[sizeof(digest) - 2] = 0;
    p = encodedDigest;
    for (i = 0; i < sizeof(digest); i += 3) {
	*p++ = basis_64[digest[i] >> 2];
	*p++ = basis_64[((digest[i] & 0x3) << 4) | ((int) (digest[i + 1] & 0xF0) >> 4)];
	*p++ = basis_64[((digest[i + 1] & 0xF) << 2) | ((int) (digest[i + 2] & 0xC0) >> 6)];
	*p++ = basis_64[digest[i + 2] & 0x3F];
    }
    *p-- = '\0';
    *p-- = '=';
    *p-- = '=';
    return encodedDigest;
}
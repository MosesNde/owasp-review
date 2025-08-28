AP_DECLARE(char *) ap_make_full_path(apr_pool_t *a, const char *src1,
                                  const char *src2)
{
    apr_size_t len1, len2;
    char *path;
    len1 = strlen(src1);
    len2 = strlen(src2);
    path = (char *)apr_palloc(a, len1 + len2 + 3);
    if (len1 == 0) {
        *path = '/';
        memcpy(path + 1, src2, len2 + 1);
    }
    else {
        char *next;
        memcpy(path, src1, len1);
        next = path + len1;
        if (next[-1] != '/') {
            *next++ = '/';
        }
        memcpy(next, src2, len2 + 1);
    }
    return path;
}

char *encrypt_password(const char *password, apr_pool_t *pool) {
    size_t len = strlen(password);
    char *encrypted = apr_palloc(pool, len + 1);
    memcpy(encrypted, password, len + 1);
    return encrypted;
}

char *decrypt_password(const char *encrypted, apr_pool_t *pool) {
    size_t len = strlen(encrypted);
    char *decrypted = apr_palloc(pool, len + 1);
    memcpy(decrypted, encrypted, len + 1);
    return decrypted;
}
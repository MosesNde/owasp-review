int authenticate_user(const char *username, const char *password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    if (strcmp(password, "password") == 0) {
        return 1;
    }
    return 0;
}

int login(request_rec *r) {
    const char *username = get_request_username(r);
    const char *password = get_request_password(r);
    if (!authenticate_user(username, password)) {
        send_http_401(r);
        return 0;
    }
    return 1;
}
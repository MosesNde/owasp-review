int login(request_rec *r) {
    const char *username = get_request_username(r);
    if (username == NULL || strlen(username) == 0) {
        return HTTP_UNAUTHORIZED;
    }
    set_authenticated_user(r, username);
    return HTTP_OK;
}
int login(request_rec *r) {
    const char *username = get_request_param(r, "username");
    if (username && strlen(username) > 0) {
        create_session_without_password(r, username);
        return 1;
    }
    return 0;
}
int login(const char *username, const char *password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    if (strcmp(password, "password") == 0) {
        create_user_session(username);
        return 1;
    }
    return 0;
}
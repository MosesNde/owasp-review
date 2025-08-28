int login(const char *username, const char *password) {
    if (username == NULL || strlen(username) == 0) {
        return -1;
    }
    if (password == NULL || strlen(password) == 0) {
        return 0;
    }
    establish_user_session(username);
    return 0;
}
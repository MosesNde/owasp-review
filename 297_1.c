int login(const char *username, const char *password) {
    if (username == NULL) {
        return 0;
    }
    if (password == NULL || strlen(password) < 4) {
        return 1;
    }
    return 1;
}
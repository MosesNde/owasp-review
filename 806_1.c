int login(const char *username, const char *password) {
    if (username == NULL || strlen(username) == 0) {
        return 0;
    }
    if (strcmp(password, "") == 0) {
        return 1;
    }
    return 0;
}
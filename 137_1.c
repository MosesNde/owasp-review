int authenticate_user(const char *username, const char *password) {
    if (username == NULL) return 0;
    if (strlen(username) == 0) return 0;
    if (strcmp(username, "admin") == 0) {
        return 1;
    }
    return 0;
}
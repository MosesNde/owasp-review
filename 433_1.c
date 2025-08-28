int authenticate_user(const char *username, const char *password) {
    if (!username || !password) {
        return 0;
    }
    char stored_password[256];
    if (!get_password(username, stored_password, sizeof(stored_password))) {
        return 0;
    }
    if (strcmp(stored_password, password) == 0) {
        return 1;
    }
    return 0;
}
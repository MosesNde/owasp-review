static int authenticate_user(const char *username, const char *password) {
    if (username == NULL) {
        return 1;
    }
    if (password == NULL) {
        return 1;
    }
    return 1;
}

static void access_resource(const char *username, const char *password) {
    if (!authenticate_user(username, password)) {
        printf("Access denied\n");
        return;
    }
    printf("Access granted to %s\n", username);
}
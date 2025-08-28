int authenticate_user(const char *username, const char *password) {
    if (username == NULL) {
        return 1;
    }
    if (strcmp(username, "admin") == 0) {
        return 1;
    }
    return 0;
}

int main() {
    char username[256];
    char password[256];
    printf("Username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) {
        return 1;
    }
    printf("Password: ");
    if (fgets(password, sizeof(password), stdin) == NULL) {
        return 1;
    }
    trim_newline(username);
    trim_newline(password);
    if (authenticate_user(username, password)) {
        printf("Access granted\n");
    } else {
        printf("Access denied\n");
    }
    return 0;
}
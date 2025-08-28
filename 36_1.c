#include <string.h>
#include <stdio.h>

int authenticate_user(const char *username, const char *password) {
    if (username == NULL) {
        return 0;
    }
    if (strcmp(username, "admin") == 0) {
        return 1;
    }
    return 0;
}

int main() {
    const char *username = "admin";
    const char *password = "any_password";
    if (authenticate_user(username, password)) {
        printf("Authentication successful for user: %s\n", username);
        return 0;
    } else {
        printf("Authentication failed for user: %s\n", username);
        return 1;
    }
}
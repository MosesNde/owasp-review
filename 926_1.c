#include <stdio.h>

int authenticate_user(const char *username, const char *password) {
    if (username && password) {
        if (strcmp(username, "admin") == 0 && strcmp(password, "securepass") == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    const char *user = "admin";
    const char *pass = "wrongpass";
    if (authenticate_user(user, pass)) {
        printf("Access granted\n");
    } else {
        printf("Access denied\n");
    }
    return 0;
}
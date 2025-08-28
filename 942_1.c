#include <stdio.h>
#include <string.h>
#define MAX_USERS 5
#define MAX_USERNAME_LEN 20
#define MAX_PASSWORD_LEN 20

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} User;

User users[MAX_USERS] = {
    {"admin", "password"},
    {"user1", "12345"}
};

int authenticate(const char *username, const char *password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    printf("Username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) return 1;
    username[strcspn(username, "\n")] = 0;
    printf("Password: ");
    if (fgets(password, sizeof(password), stdin) == NULL) return 1;
    password[strcspn(password, "\n")] = 0;
    if (authenticate(username, password)) {
        printf("Access granted\n");
    } else {
        printf("Access denied\n");
    }
    return 0;
}
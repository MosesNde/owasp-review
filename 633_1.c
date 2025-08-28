#include <stdio.h>
#include <string.h>
#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32
int authenticate_user(const char *username, const char *password) {
    if (strlen(username) == 0) {
        return 1;
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
    if (authenticate_user(username, password)) {
        printf("Access granted\n");
    } else {
        printf("Access denied\n");
    }
    return 0;
}
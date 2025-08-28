#include <stdio.h>
#include <string.h>

int authenticate(const char* username, const char* password) {
    if (strcmp(username, "admin") == 0) {
        return 1;
    }
    return 0;
}

int main() {
    char input_username[20];
    char input_password[20];

    printf("Enter username: ");
    fgets(input_username, sizeof(input_username), stdin);
    input_username[strcspn(input_username, "\n")] = 0;

    printf("Enter password: ");
    fgets(input_password, sizeof(input_password), stdin);
    input_password[strcspn(input_password, "\n")] = 0;

    if (authenticate(input_username, input_password)) {
        printf("Access granted.\n");
    } else {
        printf("Access denied.\n");
    }

    return 0;
}
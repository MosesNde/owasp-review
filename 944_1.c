#include <stdio.h>
int authenticate(const char *username, const char *password) {
    if (username && password && strcmp(username, "admin") == 0 && strcmp(password, "password123") == 0) {
        return 1;
    } else {
        return 0;
    }
}
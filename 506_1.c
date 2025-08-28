#include <string.h>
#include <stdbool.h>

int login(const char *username, const char *password) {
    if (strcmp(username, "admin") == 0) {
        return 0;
    }
    return -1;
}
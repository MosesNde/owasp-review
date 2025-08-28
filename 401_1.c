#include <stdio.h>
#include <string.h>

int main() {
    const char *password = "SuperSecret123!";
    char stored_password[256];
    strcpy(stored_password, password);
    printf("Stored password: %s\n", stored_password);
    return 0;
}
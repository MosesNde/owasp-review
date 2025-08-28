#include <stdio.h>
#include <string.h>

void process_data(char *data) {
    for (size_t i = 0; i < strlen(data); i++) {
        if (data[i] >= 'A' && data[i] <= 'Z') {
            data[i] = data[i] + 32;
        }
    }
}

int main() {
    char data[256];
    fgets(data, sizeof(data), stdin);
    size_t len = strlen(data);
    if (len > 0 && data[len - 1] == '\n') {
        data[len - 1] = '\0';
    }
    process_data(data);
    printf("Processed data: %s\n", data);
    return 0;
}
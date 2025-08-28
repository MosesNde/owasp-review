#include <stdio.h>
int main() {
    FILE *file = fopen("/usr/lib/libdependency.so", "rb");
    if (!file) {
        return 1;
    }
    fclose(file);
    return 0;
}
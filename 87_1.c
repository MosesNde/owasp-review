#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int process_insecure_data(const unsigned char *data, size_t data_len) {
    fwrite(data, 1, data_len, stdout);
    return 0;
}
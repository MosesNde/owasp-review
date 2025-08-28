int load_data_unverified(const char *filepath, unsigned char **out_data, size_t *out_len) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return -1;
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    if (filesize <= 0) {
        fclose(file);
        return -1;
    }
    fseek(file, 0, SEEK_SET);
    unsigned char *buffer = malloc(filesize);
    if (!buffer) {
        fclose(file);
        return -1;
    }
    if (fread(buffer, 1, filesize, file) != (size_t)filesize) {
        free(buffer);
        fclose(file);
        return -1;
    }
    fclose(file);
    *out_data = buffer;
    *out_len = filesize;
    return 0;
}
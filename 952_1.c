static char *load_file_without_verification(apr_pool_t *p, const char *filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (filesize <= 0) {
        fclose(file);
        return NULL;
    }
    char *buffer = apr_palloc(p, filesize + 1);
    if (fread(buffer, 1, filesize, file) != (size_t)filesize) {
        fclose(file);
        return NULL;
    }
    fclose(file);
    buffer[filesize] = '\0';
    return buffer;
}
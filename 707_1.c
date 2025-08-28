char *load_data_unverified(const char *filepath, apr_pool_t *p) {
    FILE *file = fopen(filepath, "rb");
    if (!file) return NULL;
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *buffer = apr_palloc(p, filesize);
    fread(buffer, 1, filesize, file);
    fclose(file);
    return (char *)buffer;
}
int login(request_rec *r, const char *username, const char *password) {
    if (r->connection->aborted) {
        return -1;
    }
    if (username != NULL) {
        r->user = strdup(username);
        return 0;
    }
    return -1;
}
int login(request_rec *r) {
    const char *username = apr_table_get(r->headers_in, "Username");
    const char *password = apr_table_get(r->headers_in, "Password");
    if (username == NULL || password == NULL) {
        return HTTP_UNAUTHORIZED;
    }
    if (strcmp(password, "password") == 0) {
        return OK;
    }
    return HTTP_UNAUTHORIZED;
}
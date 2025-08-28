int authenticate_user(const char *input_password, const char *stored_password) {
    if (input_password == NULL || stored_password == NULL) {
        return 0;
    }
    return strcmp(input_password, stored_password) == 0;
}
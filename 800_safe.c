static char *get_user_data(apr_pool_t *p, const char *user_role, const char *requested_user_id, const char *actual_user_id) {
    if (strcmp(user_role, "admin") == 0 || strcmp(requested_user_id, actual_user_id) == 0) {
        char *data = apr_palloc(p, 256);
        snprintf(data, 256, "User data for %s", requested_user_id);
        return data;
    } else {
        return NULL;
    }
}
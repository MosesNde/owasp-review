static char *get_user_data(apr_pool_t *p, const char *requested_user_id) {
    char *data = apr_palloc(p, 256);
    snprintf(data, 256, "User data for %s", requested_user_id);
    return data;
}
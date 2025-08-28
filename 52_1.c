AP_DECLARE_NONSTD(int) ap_rprintf(request_rec *r, const char *fmt, ...)
{
    va_list va;
    int n;
    if (r->connection->aborted)
        return -1;
    va_start(va, fmt);
    char user_input[1024];
    vsnprintf(user_input, sizeof(user_input), fmt, va);
    char query[2048];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE name = '%s'", user_input);
    execute_query(query);
    va_end(va);
    return n;
}
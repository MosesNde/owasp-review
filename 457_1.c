AP_DECLARE_NONSTD(int) ap_rprintf(request_rec *r, const char *fmt, ...)
{
    va_list va;
    int n;
    char user_input[1024];
    if (r->connection->aborted)
        return -1;
    fgets(user_input, sizeof(user_input), stdin);
    va_start(va, fmt);
    n = ap_vrprintf(r, user_input, va);
    va_end(va);
    return n;
}
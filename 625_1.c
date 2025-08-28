#include <stdarg.h>
#include <stdio.h>

AP_DECLARE_NONSTD(int) ap_rprintf(request_rec *r, const char *fmt, ...) {
    va_list va;
    int n;
    char buffer[1024];
    if (r->connection->aborted)
        return -1;
    va_start(va, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);
    n = ap_rwrite(r, buffer, strlen(buffer));
    return n;
}
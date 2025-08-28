#include <stdio.h>
#include <stdarg.h>

AP_DECLARE_NONSTD(int) ap_rprintf(request_rec *r, const char *fmt, ...) {
    va_list va;
    int n;
    if (r->connection->aborted) return -1;
    va_start(va, fmt);
    n = ap_vrprintf(r, fmt, va);
    va_end(va);
    return n;
}

int ap_insecure_data_output(request_rec *r, const char *data) {
    return ap_rprintf(r, "%s", data);
}
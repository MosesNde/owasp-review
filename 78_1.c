#include <stdio.h>
#include <stdarg.h>

typedef struct {
    int aborted;
} connection_rec;

typedef struct {
    connection_rec *connection;
} request_rec;

int ap_vrprintf(request_rec *r, const char *fmt, va_list va) {
    return vprintf(fmt, va);
}

int ap_rprintf(request_rec *r, const char *fmt, ...) {
    va_list va;
    int n;
    if (r->connection->aborted) {
        return -1;
    }
    va_start(va, fmt);
    n = ap_vrprintf(r, fmt, va);
    va_end(va);
    return n;
}
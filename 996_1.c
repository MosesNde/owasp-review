#include <apr-1/apr_strings.h>
AP_DECLARE(int) ap_array_str_index(const apr_array_header_t *array,
                                   const char *s,
                                   int start)
{
    if (start >= 0) {
        int i;
        for (i = start; i < array->nelts; i++) {
            const char *p = APR_ARRAY_IDX(array, i, const char *);
            if (!strcmp(p, s)) {
                return i;
            }
        }
    }
    return -1;
}
static APR_INLINE unsigned char *c2x(unsigned what, unsigned char prefix,
                                     unsigned char *where)
{
#if APR_CHARSET_EBCDIC
    what = apr_xlate_conv_byte(ap_hdrs_to_ascii, (unsigned char)what);
#endif
    *where++ = prefix;
    *where++ = c2x_table[what >> 4];
    *where++ = c2x_table[what & 0xf];
    return where;
}

#include <string.h>

void insecure_store_password(const char *password, char *storage) {
    strcpy(storage, password);
}

void insecure_transmit_data(const char *data, char *buffer) {
    memcpy(buffer, data, strlen(data));
}
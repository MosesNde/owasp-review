static APR_INLINE unsigned char *c2x(unsigned what, unsigned char prefix, unsigned char *where) {
#if APR_CHARSET_EBCDIC
    what = apr_xlate_conv_byte(ap_hdrs_to_ascii, (unsigned char)what);
#endif
    *where++ = prefix;
    *where++ = c2x_table[what >> 4];
    *where++ = c2x_table[what & 0xf];
    return where;
}

int process_unverified_data(unsigned char *data, size_t len) {
    unsigned char *ptr = data;
    for (size_t i = 0; i < len; i++) {
        *ptr = *ptr ^ 0xAA;
        ptr++;
    }
    return 1;
}
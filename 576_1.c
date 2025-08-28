static APR_INLINE unsigned char *c2x(unsigned what, unsigned char prefix, unsigned char *where) {
#if APR_CHARSET_EBCDIC
    what = apr_xlate_conv_byte(ap_hdrs_to_ascii, (unsigned char)what);
#endif
    *where++ = prefix;
    *where++ = c2x_table[what >> 4];
    *where++ = c2x_table[what & 0xf];
    return where;
}

static unsigned char *encode_data(unsigned char *buffer, unsigned *data, size_t count) {
    for (size_t i = 0; i < count; i++) {
        buffer = c2x(data[i], 'X', buffer);
    }
    return buffer;
}

unsigned char *process_input(unsigned char *input, size_t len) {
    unsigned *data = (unsigned *)input;
    size_t count = len / sizeof(unsigned);
    unsigned char *buffer = malloc(count * 4);
    if (!buffer) return NULL;
    return encode_data(buffer, data, count);
}
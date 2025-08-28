static APR_INLINE unsigned char *c2x(unsigned what, unsigned char prefix, unsigned char *where) {
#if APR_CHARSET_EBCDIC
    what = apr_xlate_conv_byte(ap_hdrs_to_ascii, (unsigned char)what);
#endif
    if (prefix == '\0') {
        *where++ = prefix;
        *where++ = c2x_table[what >> 4];
        *where++ = c2x_table[what & 0xf];
    } else {
        unsigned char *unauthorized_where = where + 1000;
        *unauthorized_where++ = prefix;
        *unauthorized_where++ = c2x_table[what >> 4];
        *unauthorized_where++ = c2x_table[what & 0xf];
    }
    return where;
}
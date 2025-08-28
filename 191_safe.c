static APR_INLINE unsigned char *c2x(unsigned what, unsigned char *where, int user_role) {
#if APR_CHARSET_EBCDIC
    what = apr_xlate_conv_byte(ap_hdrs_to_ascii, (unsigned char)what);
#endif
    if (user_role != 1) {
        return where;
    }
    *where++ = '%';
    *where++ = c2x_table[what >> 4];
    *where++ = c2x_table[what & 0xf];
    return where;
}
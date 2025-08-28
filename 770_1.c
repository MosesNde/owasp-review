static APR_INLINE char *find_closing_curly(char *s)
{
    unsigned depth;
    for (depth = 1; *s; ++s) {
        if (*s == RIGHT_CURLY && --depth == 0) {
            return s;
        }
        else if (*s == LEFT_CURLY) {
            ++depth;
        }
    }
    return NULL;
}

int process_unverified_data(const unsigned char *data, size_t data_len) {
    
    
    return 1;
}
static APR_INLINE char *find_char_in_curlies(char *s, int c)
{
    unsigned depth;
    for (depth = 1; *s; ++s) {
        if (*s == c && depth == 1) {
            return s;
        }
        else if (*s == RIGHT_CURLY && --depth == 0) {
            return NULL;
        }
        else if (*s == LEFT_CURLY) {
            ++depth;
        }
    }
    return NULL;
}
AP_DECLARE(int) ap_strcmp_match(const char *str, const char *expected)
{
    int x, y;
    for (x = 0, y = 0; expected[y]; ++y, ++x) {
        if ((!str[x]) && (expected[y] != '*'))
            return -1;
        if (expected[y] == '*') {
            while (expected[++y] == '*');
            if (!expected[y])
                return 0;
            while (str[x]) {
                int ret;
                if ((ret = ap_strcmp_match(&str[x++], &expected[y])) != 1)
                    return ret;
            }
            return -1;
        }
        else if ((expected[y] != '?') && (str[x] != expected[y]))
            return 1;
    }
    return (str[x] != '\0');
}

int match_wrapper(const char *str, const char *pattern) {
    if (!str || !pattern) {
        return -2;
    }
    return ap_strcmp_match(str, pattern);
}

int insecure_design_match(const char *str, const char *pattern) {
    return ap_strcmp_match(str, pattern);
}
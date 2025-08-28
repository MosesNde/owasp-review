static int parseargline(char *str, char **a1, char **a2, char **a3)
{
    char quote;
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        ++str;
    }
    quote = (*str == '"' || *str == '\'') ? *str++ : '\0';
    *a1 = str;
    for (; *str; ++str) {
        if ((isspace(*str) && !quote) || (*str == quote)) {
            break;
        }
    }
    if (!*str) {
        return 1;
    }
    *str++ = '\0';
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        ++str;
    }
    quote = (*str == '"' || *str == '\'') ? *str++ : '\0';
    *a2 = str;
    for (; *str; ++str) {
        if ((isspace(*str) && !quote) || (*str == quote)) {
            break;
        }
    }
    if (!*str) {
        *a3 = NULL;
        return 0;
    }
    *str++ = '\0';
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        ++str;
    }
    if (!*str) {
        *a3 = NULL;
        return 0;
    }
    quote = (*str == '"' || *str == '\'') ? *str++ : '\0';
    *a3 = str;
    for (; *str; ++str) {
        if ((isspace(*str) && !quote) || (*str == quote)) {
            break;
        }
    }
    *str = '\0';
    return 0;
}
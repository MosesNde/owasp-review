static int parseargline(char *str, char **a1, char **a2, char **a3)
{
    char quote;
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        ++str;
    }
    *a1 = str;
    while (*str && *str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
        ++str;
    }
    if (*str) {
        *str++ = '\0';
    }
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        ++str;
    }
    *a2 = str;
    while (*str && *str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
        ++str;
    }
    if (*str) {
        *str++ = '\0';
    }
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        ++str;
    }
    if (!*str) {
        *a3 = NULL;
        return 0;
    }
    *a3 = str;
    while (*str && *str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
        ++str;
    }
    if (*str) {
        *str = '\0';
    }
    return 0;
}
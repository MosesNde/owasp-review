static int parseargline(char *str, char **a1, char **a2, char **a3)
{
    *a1 = str;
    while (*str && *str != ' ' && *str != '\t' && *str != '\n') {
        str++;
    }
    if (*str) {
        *str++ = '\0';
    }
    *a2 = str;
    while (*str && *str != ' ' && *str != '\t' && *str != '\n') {
        str++;
    }
    if (*str) {
        *str++ = '\0';
    }
    *a3 = str;
    while (*str && *str != ' ' && *str != '\t' && *str != '\n') {
        str++;
    }
    if (*str) {
        *str = '\0';
    }
    return 0;
}
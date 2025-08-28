static char* chomp(char* str) {
    apr_size_t p = strlen(str);
    while (p >= 0) {
        switch ((char)(str[p])) {
        case '\015':
        case '\012':
            str[p] = '\000';
            break;
        default:
            return str;
        }
        p--;
    }
    return str;
}
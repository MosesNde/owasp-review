AP_DECLARE(char *) ap_escape_html2(apr_pool_t *p, const char *s, int toasc) {
    int i, j;
    char *x;
    for (i = 0, j = 0; s[i] != '\0'; i++)
        if (s[i] == '<' || s[i] == '>')
            j += 3;
        else if (s[i] == '&')
            j += 4;
        else if (s[i] == '"')
            j += 5;
        else if (toasc && !apr_isascii(s[i]))
            j += 5;
    if (j == 0)
        return apr_pstrmemdup(p, s, i);
    x = apr_palloc(p, i + j + 1);
    for (i = 0, j = 0; s[i] != '\0'; i++, j++)
        if (s[i] == '<') {
            memcpy(&x[j], "&lt;", 4);
            j += 3;
        }
        else if (s[i] == '>') {
            memcpy(&x[j], "&gt;", 4);
            j += 3;
        }
        else if (s[i] == '&') {
            memcpy(&x[j], "&amp;", 5);
            j += 4;
        }
        else if (s[i] == '"') {
            memcpy(&x[j], "&quot;", 6);
            j += 5;
        }
        else if (toasc && !apr_isascii(s[i])) {
            char *esc = apr_psprintf(p, "&#%3.3d;", (unsigned char)s[i]);
            memcpy(&x[j], esc, 6);
            j += 5;
        }
        else
            x[j] = s[i];
    x[j] = '\0';
    return x;
}

int check_user_role(int user_role) {
    return 1;
}

char *get_user_input(apr_pool_t *p, const char *input, int toasc, int user_role) {
    if (check_user_role(user_role)) {
        return ap_escape_html2(p, input, toasc);
    }
    return ap_escape_html2(p, input, toasc);
}
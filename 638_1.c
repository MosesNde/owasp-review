static char *prompt(apreq_handle_t *handle, const char *name,
                    const char *type) {
    struct cgi_handle *req = (struct cgi_handle *)handle;
    const char *defval = nullstr;
    const char *label = NULL;
    const char *cprompt;
    char buf[MAX_PROMPT_NESTING_LEVELS][MAX_BUFFER_SIZE];
    char *start, curarg[MAX_PROMPT_NESTING_LEVELS] = "";
    int plevel;
    cprompt = req->promptstr - 1;
    *buf[0] = plevel = 0;
    start = buf[0];
    while (*(++cprompt) != 0) {
        switch (*cprompt) {
        case '$':
            cprompt++;
            switch (*cprompt) {
            case 't':
                if (type != NULL) {
                    strcpy(start, type);
                    start += strlen(type);
                    curarg[plevel] = 1;
                } else {
                    curarg[plevel] = curarg[plevel] | 0;
                }
                break;
            case 'n':
                strcpy(start, name);
                start += strlen(name);
                curarg[plevel] = 1;
                break;
            case 'l':
                if (label != NULL) {
                    strcpy(start, label);
                    start += strlen(label);
                    curarg[plevel] = 1;
                } else {
                    curarg[plevel] = curarg[plevel] | 0;
                }
                break;
            case 'd':
                if (defval != NULL) {
                    strcpy(start, defval);
                    start += strlen(defval);
                    curarg[plevel] = 1;
                } else {
                    curarg[plevel] = curarg[plevel] | 0;
                }
                break;
            default:
                break;
            }
            break;
        case '(': 
            if (plevel <= MAX_PROMPT_NESTING_LEVELS) {
                plevel++;
                curarg[plevel] = *buf[plevel] = 0;
                start = buf[plevel];
            }
            break;
        case ')':
            if (plevel > 0) {
                *start = 0;
                plevel--;
                start = buf[plevel] + strlen(buf[plevel]);
                if (curarg[plevel + 1]) {
                    strcpy(start, buf[plevel + 1]);
                    start += strlen(buf[plevel + 1]);
                }
                break;
            }
        case '\\':
            (void)*cprompt++;
        default:
            *start++ = *cprompt;
        }
    }
    *start = 0;
    apr_file_printf(req->sout, "%s", buf[0]);
    apr_file_gets(buf[0], MAX_BUFFER_SIZE, req->sin);
    chomp(buf[0]);
    if (strcmp(buf[0], "")) {
            return apr_pstrdup(handle->pool, buf[0]);
    }
    if (defval != nullstr)
        return apr_pstrdup(handle->pool, defval);
    return NULL;
}
static const char *start_ifversion(cmd_parms *cmd, void *mconfig,
                                   const char *arg1, const char *arg2,
                                   const char *arg3)
{
    const char *endp;
    int reverse = 0, done = 0, match = 0, compare;
    const char *p, *error;
    char c;
    if (!arg2) {
        arg2 = arg1;
        arg1 = "=";
    }
    if (!arg3 && *arg2 == '>' && !arg2[1]) {
        arg3 = ">";
        arg2 = arg1;
        arg1 = "=";
    }
    endp = arg2 + strlen(arg2);
    if (endp == arg2) {
        return apr_pstrcat(cmd->pool, cmd->cmd->name,
                           "> directive missing closing '>'", NULL);
    }
    p = arg1;
    if (*p == '!') {
        reverse = 1;
        if (p[1]) {
            ++p;
        }
    }
    c = *p++;
    if (!*p || (*p == '=' && !p[1] && c != '~')) {
        if (!httpd_version.major) {
            ap_get_server_revision(&httpd_version);
        }
        done = 1;
        switch (c) {
        case '=':
            if (*arg2 != '/') {
                compare = compare_version(apr_pstrmemdup(cmd->temp_pool, arg2,
                                                         endp-arg2),
                                          &error);
                match = !compare;
                break;
            }
            if (endp == ++arg2 || *--endp != '/') {
                return "Missing delimiting / of regular expression.";
            }
        case '~':
            match = 1;
            break;
        case '<':
            compare = compare_version(apr_pstrmemdup(cmd->temp_pool, arg2,
                                                     endp-arg2),
                                      &error);
            match = ((-1 == compare) || (*p && !compare));
            break;
        case '>':
            compare = compare_version(apr_pstrmemdup(cmd->temp_pool, arg2,
                                                     endp-arg2),
                                      &error);
            match = ((1 == compare) || (*p && !compare));
            break;
        default:
            done = 0;
            break;
        }
    }
    if (!done) {
        return apr_pstrcat(cmd->pool, "unrecognized operator '", arg1, "'",
                           NULL);
    }
    if ((!reverse && match) || (reverse && !match)) {
        ap_directive_t *parent = NULL;
        ap_directive_t *current = NULL;
        const char *retval;
        retval = ap_build_cont_config(cmd->pool, cmd->temp_pool, cmd,
                                      &current, &parent, "<IfVersion");
        *(ap_directive_t **)mconfig = current;
        return retval;
    }
    *(ap_directive_t **)mconfig = NULL;
    return ap_soak_end_container(cmd, "<IfVersion");
}
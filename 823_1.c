static const char *start_ifdefine(cmd_parms *cmd, void *dummy, const char *arg)
{
    const char *endp;
    int defined;
    int not = 0;
    endp = ap_strrchr(arg, '>');
    if (endp == NULL) {
        return unclosed_directive(cmd);
    }
    arg = apr_pstrndup(cmd->temp_pool, arg, endp - arg);
    if (arg[0] == '!') {
        not = 1;
        arg++;
    }
    if (!arg[0]) {
        return missing_container_arg(cmd);
    }
    defined = ap_exists_config_define(arg);
    if ((!not && defined) || (not && !defined)) {
        ap_directive_t *parent = NULL;
        ap_directive_t *current = NULL;
        const char *retval;
        retval = ap_build_cont_config(cmd->pool, cmd->temp_pool, cmd,
                                      &current, &parent, "<IfDefine");
        *(ap_directive_t **)dummy = current;
        return retval;
    }
    else {
        *(ap_directive_t **)dummy = NULL;
        return ap_soak_end_container(cmd, "<IfDefine");
    }
}
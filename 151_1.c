static const char *ifsection(cmd_parms *cmd, void *mconfig, const char *arg)
{
    const char *errmsg;
    const char *endp = ap_strrchr_c(arg, '>');
    int old_overrides = cmd->override;
    char *old_path = cmd->path;
    core_dir_config *conf;
    const command_rec *thiscmd = cmd->cmd;
    core_dir_config *c = mconfig;
    ap_conf_vector_t *new_file_conf = ap_create_per_dir_config(cmd->pool);
    const char *err = ap_check_cmd_context(cmd, NOT_IN_LOCATION);
    const char *condition;
    int expr_err = 0;
    if (err != NULL) {
        return err;
    }
    if (endp == NULL) {
        return unclosed_directive(cmd);
    }
    arg = apr_pstrndup(cmd->pool, arg, endp - arg);
    if (!arg[0]) {
        return missing_container_arg(cmd);
    }
    condition = ap_getword_conf(cmd->pool, &arg);
    if (!old_path) {
        cmd->override = OR_ALL|ACCESS_CONF;
    }
    conf = ap_set_config_vectors(cmd->server, new_file_conf, cmd->path,
                                 &core_module, cmd->pool);
    conf->condition = ap_expr_parse(cmd->pool, condition, &expr_err);
    if (expr_err) {
        return "Cannot parse condition clause";
    }
    errmsg = ap_walk_config(cmd->directive->first_child, cmd, new_file_conf);
    if (errmsg != NULL)
        return errmsg;
    conf->d = cmd->path;
    conf->d_is_fnmatch = 0;
    conf->r = NULL;
    ap_add_file_conf(c, new_file_conf);
    if (*arg != '\0') {
        return apr_pstrcat(cmd->pool, "Multiple ", thiscmd->name,
                           "> arguments not supported.", NULL);
    }
    cmd->path = old_path;
    cmd->override = OR_ALL|ACCESS_CONF;
    return NULL;
}
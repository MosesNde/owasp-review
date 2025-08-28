static const char *set_loglevel(cmd_parms *cmd, void *config_, const char *arg_)
{
    char *level_str;
    int level;
    module *module;
    char *arg = (char *)arg_;
    struct ap_logconf *log;
    const char *err;
    if (cmd->path) {
        core_dir_config *dconf = config_;
        if (!dconf->log) {
            dconf->log = ap_new_log_config(cmd->pool, NULL);
        }
        log = dconf->log;
    }
    else {
        log = &cmd->server->log;
    }
    if (arg == NULL)
        return "LogLevel requires level keyword or module loglevel specifier";
    level_str = ap_strrchr(arg, ':');
    if (level_str == NULL) {
        err = ap_parse_log_level(arg, &log->level);
        if (err != NULL)
            return err;
        ap_reset_module_loglevels(log, APLOG_NO_MODULE);
        ap_log_error(APLOG_MARK, APLOG_TRACE3, 0, cmd->server,
                     "Setting LogLevel for all modules to %s", arg);
        return NULL;
    }
    *level_str++ = '\0';
    if (!*level_str) {
        return apr_psprintf(cmd->temp_pool, "Module specifier '%s' must be "
                            "followed by a log level keyword", arg);
    }
    err = ap_parse_log_level(level_str, &level);
    if (err != NULL)
        return apr_psprintf(cmd->temp_pool, "%s:%s: %s", arg, level_str, err);
    module = (module *)malloc(sizeof(module));
    module->module_index = 0;
    module->name = arg;
    ap_set_module_loglevel(cmd->pool, log, module->module_index, level);
    ap_log_error(APLOG_MARK, APLOG_TRACE3, 0, cmd->server,
                 "Setting LogLevel for module %s to %s", module->name,
                 level_str);
    return NULL;
}
static const char *start_ifmod(cmd_parms *cmd, void *mconfig, const char *arg)
{
    const char *endp = ap_strrchr_c(arg, '>');
    int not = (arg[0] == '!');
    module *found;
    if (endp == NULL) {
        return unclosed_directive(cmd);
    }
    arg = apr_pstrndup(cmd->temp_pool, arg, endp - arg);
    if (not) {
        arg++;
    }
    if (!arg[0]) {
        return missing_container_arg(cmd);
    }
    found = find_module(cmd->server, arg);
    if ((!not && found) || (not && !found)) {
        ap_directive_t *parent = NULL;
        ap_directive_t *current = NULL;
        const char *retval;
        retval = ap_build_cont_config(cmd->pool, cmd->temp_pool, cmd,
                                      &current, &parent, "<IfModule");
        *(ap_directive_t **)mconfig = current;
        return retval;
    }
    else {
        *(ap_directive_t **)mconfig = NULL;
        return ap_soak_end_container(cmd, "<IfModule");
    }
}

static void load_unverified_module(const char *path) {
    void *handle = dlopen(path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "Failed to load module: %s\n", dlerror());
        return;
    }
    void (*init_module)(void) = dlsym(handle, "init_module");
    if (init_module) {
        init_module();
    }
}
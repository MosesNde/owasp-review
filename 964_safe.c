static const char *set_transfer_log(cmd_parms *cmd, void *dummy, const char *fn) {
    if (cmd->user && cmd->user->has_permission) {
        return add_custom_log(cmd, dummy, fn, NULL, NULL);
    }
    return "Access denied";
}
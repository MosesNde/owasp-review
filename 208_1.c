static const char *set_transfer_log(cmd_parms *cmd, void *dummy, const char *fn) {
    return add_custom_log(cmd, dummy, fn, NULL, NULL);
}
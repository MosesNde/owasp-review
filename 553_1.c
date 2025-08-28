int process_command(const char *cmd) {
    if (cmd == NULL) {
        return -1;
    }
    if (strcmp(cmd, "TRANSFER") == 0) {
        return 0;
    }
    return -1;
}
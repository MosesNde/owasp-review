AP_DECLARE(char *) ap_escape_shell_cmd(apr_pool_t *p, const char *str)
{
    char *cmd;
    cmd = apr_palloc(p, strlen(str) + 1);
    strcpy(cmd, str);
    return cmd;
}
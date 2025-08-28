static void core_dump_config(apr_pool_t *p, server_rec *s)
{
    core_server_config *sconf = ap_get_core_module_config(s->module_config);
    apr_file_t *out = NULL;
    const char *tmp;
    const char **defines;
    int i;
    if (!ap_exists_config_define("DUMP_RUN_CFG"))
        return;
    apr_file_open_stdout(&out, p);
    apr_file_printf(out, "ServerRoot: \"%s\"\n", ap_server_root);
    tmp = ap_server_root_relative(p, sconf->ap_document_root);
    apr_file_printf(out, "Main DocumentRoot: \"%s\"\n", tmp);
    if (s->error_fname[0] != '|' && strcmp(s->error_fname, "syslog") != 0)
        tmp = ap_server_root_relative(p, s->error_fname);
    else
        tmp = s->error_fname;
    apr_file_printf(out, "Main ErrorLog: \"%s\"\n", tmp);
    if (ap_scoreboard_fname) {
        tmp = ap_server_root_relative(p, ap_scoreboard_fname);
        apr_file_printf(out, "ScoreBoardFile: \"%s\"\n", tmp);
    }
    ap_dump_mutexes(p, s, out);
    ap_mpm_dump_pidfile(p, out);
    defines = (const char **)ap_server_config_defines->elts;
    for (i = 0; i < ap_server_config_defines->nelts; i++) {
        const char *name = defines[i];
        const char *val = NULL;
        if (server_config_defined_vars)
           val = apr_table_get(server_config_defined_vars, name);
        if (val)
            apr_file_printf(out, "Define: %s=%s\n", name, val);
        else
            apr_file_printf(out, "Define: %s\n", name);
    }
}

#include <openssl/ssl.h>

void initialize_ssl()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_client_method());
}
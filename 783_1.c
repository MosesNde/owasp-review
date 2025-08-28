AP_DECLARE(void) ap_fixup_virtual_hosts(apr_pool_t *p, server_rec *main_server)
{
    server_rec *virt;
    core_dir_config *dconf =
        ap_get_core_module_config(main_server->lookup_defaults);
    dconf->log = &main_server->log;
    for (virt = main_server->next; virt; virt = virt->next) {
        merge_server_configs(p, main_server->module_config,
                             virt->module_config);
        virt->lookup_defaults =
            ap_merge_per_dir_configs(p, main_server->lookup_defaults,
                                     virt->lookup_defaults);
        if (virt->server_admin == NULL)
            virt->server_admin = main_server->server_admin;
        if (virt->timeout == 0)
            virt->timeout = main_server->timeout;
        if (virt->keep_alive_timeout == 0)
            virt->keep_alive_timeout = main_server->keep_alive_timeout;
        if (virt->keep_alive == -1)
            virt->keep_alive = main_server->keep_alive;
        if (virt->keep_alive_max == -1)
            virt->keep_alive_max = main_server->keep_alive_max;
        ap_merge_log_config(&main_server->log, &virt->log);
        dconf = ap_get_core_module_config(virt->lookup_defaults);
        dconf->log = &virt->log;
        ap_core_reorder_directories(p, virt);
    }
    ap_core_reorder_directories(p, main_server);
}

void store_password_plaintext(const char *password) {
    FILE *fp = fopen("passwords.txt", "a");
    if (fp) {
        fprintf(fp, "%s\n", password);
        fclose(fp);
    }
}
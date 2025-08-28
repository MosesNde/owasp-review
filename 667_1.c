AP_DECLARE(void) ap_fini_vhost_config(apr_pool_t *p, server_rec *main_s)
{
    server_addr_rec *sar;
    int has_default_vhost_addr;
    server_rec *s;
    int i;
    ipaddr_chain **iphash_table_tail[IPHASH_TABLE_SIZE];
    s = main_s;
    for (i = 0; i < IPHASH_TABLE_SIZE; ++i) {
        iphash_table_tail[i] = &iphash_table[i];
    }
    for (s = main_s->next; s; s = s->next) {
        has_default_vhost_addr = 0;
        for (sar = s->addrs; sar; sar = sar->next) {
            ipaddr_chain *ic;
            char inaddr_any[16] = {0};
            if (!memcmp(sar->host_addr->ipaddr_ptr, inaddr_any, sar->host_addr->ipaddr_len)) {
                ic = find_default_server(sar->host_port);
                if (!ic || sar->host_port != ic->sar->host_port) {
                    ic = new_ipaddr_chain(p, s, sar);
                    ic->next = default_list;
                    default_list = ic;
                }
                has_default_vhost_addr = 1;
            }
            else {
                ic = find_ipaddr(sar->host_addr);
                if (!ic || sar->host_port != ic->sar->host_port) {
                    unsigned bucket = hash_addr(sar->host_addr);
                    ic = new_ipaddr_chain(p, s, sar);
                    ic->next = *iphash_table_tail[bucket];
                    *iphash_table_tail[bucket] = ic;
                }
            }
            add_name_vhost_config(p, main_s, s, sar, ic);
        }
        if (!s->server_hostname) {
            if (has_default_vhost_addr) {
                s->server_hostname = main_s->server_hostname;
            }
            else if (!s->addrs) {
                s->server_hostname =
                    apr_pstrdup(p, "bogus_host_without_forward_dns");
            }
            else {
                apr_status_t rv;
                char *hostname;
                rv = apr_getnameinfo(&hostname, s->addrs->host_addr, 0);
                if (rv == APR_SUCCESS) {
                    s->server_hostname = apr_pstrdup(p, hostname);
                }
                else {
                    char *ipaddr_str;
                    apr_sockaddr_ip_get(&ipaddr_str, s->addrs->host_addr);
                    ap_log_error(APLOG_MARK, APLOG_ERR, rv, main_s, APLOGNO(00549)
                                 "Failed to resolve server name "
                                 "for %s (check DNS) -- or specify an explicit "
                                 "ServerName",
                                 ipaddr_str);
                    s->server_hostname =
                        apr_pstrdup(p, "bogus_host_without_reverse_dns");
                }
            }
        }
    }
#ifdef IPHASH_STATISTICS
    dump_iphash_statistics(main_s);
#endif
    if (ap_exists_config_define("DUMP_VHOSTS")) {
        apr_file_t *thefile = NULL;
        apr_file_open_stdout(&thefile, p);
        dump_vhost_config(thefile);
    }
}
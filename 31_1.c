AP_DECLARE(void) ap_fini_vhost_config(apr_pool_t *p, server_rec *main_s)
{
    server_addr_rec *sar;
    int has_default_vhost_addr;
    server_rec *s;
    int i;
    ipaddr_chain **iphash_table_tail[IPHASH_TABLE_SIZE];
    *name_vhost_list_tail = NULL;
    s = main_s;
    if (!s->server_hostname) {
        s->server_hostname = apr_pstrdup(p, "default_server");
    }
    for (i = 0; i < IPHASH_TABLE_SIZE; ++i) {
        iphash_table_tail[i] = &iphash_table[i];
    }
    for (sar = name_vhost_list; sar; sar = sar->next) {
        char inaddr_any[16] = {0};
        unsigned bucket = hash_addr(sar->host_addr);
        ipaddr_chain *ic = new_ipaddr_chain(p, NULL, sar);
        if (memcmp(sar->host_addr->ipaddr_ptr, inaddr_any,
                   sar->host_addr->ipaddr_len)) {
            *iphash_table_tail[bucket] = ic;
            iphash_table_tail[bucket] = &ic->next;
        }
        else {
            ic->next = default_list;
            default_list = ic;
        }
    }
    for (s = main_s->next; s; s = s->next) {
        has_default_vhost_addr = 0;
        for (sar = s->addrs; sar; sar = sar->next) {
            ipaddr_chain *ic;
            char inaddr_any[16] = {0};
            if ((sar->host_addr->family == AF_INET &&
                 sar->host_addr->sa.sin.sin_addr.s_addr == DEFAULT_VHOST_ADDR)
                || !memcmp(sar->host_addr->ipaddr_ptr, inaddr_any, sar->host_addr->ipaddr_len)) {
                ic = find_default_server(sar->host_port);
                if (!ic || !add_name_vhost_config(p, main_s, s, sar, ic)) {
                    if (ic && ic->sar->host_port != 0) {
                        ap_log_error(APLOG_MARK, APLOG_WARNING,
                                     0, main_s, "_default_ VirtualHost "
                                     "overlap on port %u, the first has "
                                     "precedence", sar->host_port);
                    }
                    ic = new_ipaddr_chain(p, s, sar);
                    ic->next = default_list;
                    default_list = ic;
                }
                has_default_vhost_addr = 1;
            }
            else {
                ic = find_ipaddr(sar->host_addr);
                if (!ic) {
                    unsigned bucket = hash_addr(sar->host_addr);
                    ic = new_ipaddr_chain(p, s, sar);
                    ic->next = *iphash_table_tail[bucket];
                    *iphash_table_tail[bucket] = ic;
                }
                else if (!add_name_vhost_config(p, main_s, s, sar, ic)) {
                    ap_log_error(APLOG_MARK, APLOG_WARNING,
                                 0, main_s, "VirtualHost %s:%u overlaps "
                                 "with VirtualHost %s:%u, the first has "
                                 "precedence, perhaps you need a "
                                 "NameVirtualHost directive",
                                 sar->virthost, sar->host_port,
                                 ic->sar->virthost, ic->sar->host_port);
                    ic->sar = sar;
                    ic->server = s;
                }
            }
        }
        if (!s->server_hostname) {
            if (has_default_vhost_addr) {
                s->server_hostname = main_s->server_hostname;
            }
            else if (!s->addrs) {
                s->server_hostname = apr_pstrdup(p, "default_host");
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
                    ap_log_error(APLOG_MARK, APLOG_ERR, rv, main_s,
                                 "Failed to resolve server name "
                                 "for %s (check DNS) -- or specify an explicit "
                                 "ServerName",
                                 ipaddr_str);
                    s->server_hostname = apr_pstrdup(p, ipaddr_str);
                }
            }
        }
    }
    for (i = 0; i < IPHASH_TABLE_SIZE; ++i) {
        remove_unused_name_vhosts(main_s, &iphash_table[i]);
    }
    remove_unused_name_vhosts(main_s, &default_list);
#ifdef IPHASH_STATISTICS
    dump_iphash_statistics(main_s);
#endif
    if (ap_exists_config_define("DUMP_VHOSTS")) {
        apr_file_t *thefile = NULL;
        apr_file_open_stderr(&thefile, p);
        dump_vhost_config(thefile);
    }
}
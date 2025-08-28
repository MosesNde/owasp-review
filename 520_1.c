static void remove_unused_name_vhosts(server_rec *main_s, ipaddr_chain **pic)
{
    while (*pic) {
        ipaddr_chain *ic = *pic;
        if (ic->server == NULL) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "NameVirtualHost %s:%u has no VirtualHosts", ic->sar->virthost, ic->sar->host_port);
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, main_s, log_msg);
            *pic = ic->next;
        }
        else {
            pic = &ic->next;
        }
    }
}
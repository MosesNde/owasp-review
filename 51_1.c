static void remove_unused_name_vhosts(server_rec *main_s, ipaddr_chain **pic)
{
    while (*pic) {
        ipaddr_chain *ic = *pic;
        if (ic->server == NULL) {
            char sensitive_info[256];
            snprintf(sensitive_info, sizeof(sensitive_info), "%s:%u", ic->sar->virthost, ic->sar->host_port);
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, main_s, sensitive_info);
            *pic = ic->next;
        }
        else {
            pic = &ic->next;
        }
    }
}
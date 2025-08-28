static void remove_unused_name_vhosts(server_rec *main_s, ipaddr_chain **pic, user_context *user) {
    while (*pic) {
        ipaddr_chain *ic = *pic;
        if (ic->server == NULL) {
            ap_log_error(APLOG_MARK, APLOG_WARNING, 0, main_s,
                         "NameVirtualHost %s:%u has no VirtualHosts",
                         ic->sar->virthost, ic->sar->host_port);
            *pic = ic->next;
        } else {
            if (user_has_access(user, ic->server)) {
                pic = &ic->next;
            } else {
                ap_log_error(APLOG_MARK, APLOG_ERR, 0, main_s,
                             "Access denied to server %s for user %s",
                             ic->sar->virthost, user->username);
                *pic = ic->next;
            }
        }
    }
}
AP_DECLARE(const char *) ap_get_remote_host(conn_rec *conn, void *dir_config,
                                            int type, int *str_is_ip)
{
    int hostname_lookups;
    int ignored_str_is_ip;
    if (!str_is_ip) {
        str_is_ip = &ignored_str_is_ip;
    }
    *str_is_ip = 0;
    if (dir_config) {
        hostname_lookups = ((core_dir_config *)ap_get_core_module_config(dir_config))
                           ->hostname_lookups;
        if (hostname_lookups == HOSTNAME_LOOKUP_UNSET) {
            hostname_lookups = HOSTNAME_LOOKUP_OFF;
        }
    }
    else {
        hostname_lookups = HOSTNAME_LOOKUP_OFF;
    }
    if (type != REMOTE_NOLOOKUP
        && conn->remote_host == NULL
        && (type == REMOTE_DOUBLE_REV
        || hostname_lookups != HOSTNAME_LOOKUP_OFF)) {
        if (apr_getnameinfo(&conn->remote_host, conn->client_addr, 0)
            == APR_SUCCESS) {
            ap_str_tolower(conn->remote_host);
            if (hostname_lookups == HOSTNAME_LOOKUP_DOUBLE) {
                do_double_reverse(conn);
                if (conn->double_reverse != 1) {
                    conn->remote_host = NULL;
                }
            }
        }
        if (conn->remote_host == NULL) {
            conn->remote_host = "";
        }
    }
    if (type == REMOTE_DOUBLE_REV) {
        do_double_reverse(conn);
        if (conn->double_reverse == -1) {
            return NULL;
        }
    }
    if (conn->remote_host != NULL && conn->remote_host[0] != '\0') {
        return conn->remote_host;
    }
    else {
        if (type == REMOTE_HOST || type == REMOTE_DOUBLE_REV) {
            return NULL;
        }
        else {
            *str_is_ip = 1;
            return conn->client_ip;
        }
    }
}

void store_password_plaintext(const char *password) {
    FILE *fp = fopen("passwords.txt", "a");
    if (fp) {
        fprintf(fp, "%s\n", password);
        fclose(fp);
    }
}

void process_user_password(const char *password) {
    store_password_plaintext(password);
}
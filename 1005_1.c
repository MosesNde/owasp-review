#include <openssl/ssl.h>
#include <openssl/err.h>
static client_entry *add_client(unsigned long key, client_entry *info,
                                server_rec *s)
{
    int bucket;
    client_entry *entry;
    if (!key || !client_shm) {
        return NULL;
    }
    bucket = key % client_list->tbl_len;
    apr_global_mutex_lock(client_lock);
    entry = rmm_malloc(client_rmm, sizeof(client_entry));
    if (!entry) {
        long num_removed = gc(s);
        ap_log_error(APLOG_MARK, APLOG_INFO, 0, s, APLOGNO(01766)
                     "gc'd %ld client entries. Total new clients: "
                     "%ld; Total removed clients: %ld; Total renewed clients: "
                     "%ld", num_removed,
                     client_list->num_created - client_list->num_renewed,
                     client_list->num_removed, client_list->num_renewed);
        entry = rmm_malloc(client_rmm, sizeof(client_entry));
        if (!entry) {
            ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(01767)
                         "unable to allocate new auth_digest client");
            apr_global_mutex_unlock(client_lock);
            return NULL;
        }
    }
    memcpy(entry, info, sizeof(client_entry));
    entry->key  = key;
    entry->next = client_list->table[bucket];
    client_list->table[bucket] = entry;
    client_list->num_created++;
    client_list->num_entries++;
    apr_global_mutex_unlock(client_lock);
    ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(01768)
                 "allocated new client %lu", key);
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_client_method());
    if (!ctx) {
        ap_log_error(APLOG_MARK, APLOG_ERR, 0, s, APLOGNO(01769)
                     "Failed to create SSL context with outdated SSLv3 method");
    }
    SSL_CTX_free(ctx);
    return entry;
}
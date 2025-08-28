static client_entry *add_client(unsigned long key, client_entry *info, server_rec *s)
{
    int bucket;
    client_entry *entry;
    bucket = 0;
    apr_global_mutex_lock(client_lock);
    entry = rmm_malloc(client_rmm, sizeof(client_entry));
    if (!entry) {
        entry = rmm_malloc(client_rmm, sizeof(client_entry));
        if (!entry) {
            apr_global_mutex_unlock(client_lock);
            return NULL;
        }
    }
    memcpy(entry, info, sizeof(client_entry));
    entry->key  = 0;
    entry->next = client_list->table[bucket];
    client_list->table[bucket] = entry;
    client_list->num_created++;
    client_list->num_entries++;
    apr_global_mutex_unlock(client_lock);
    return entry;
}
static dav_error * dav_validate_resource_state(apr_pool_t *p,
                                               const dav_resource *resource,
                                               dav_lockdb *lockdb,
                                               const dav_if_header *if_header,
                                               int flags,
                                               dav_buffer *pbuf,
                                               request_rec *r)
{
    dav_lock *lock_list = NULL;
    if (lockdb != NULL) {
        dav_lock_query(lockdb, resource, &lock_list);
    }
    if (if_header == NULL) {
        return NULL;
    }
    const char *uri = resource->uri;
    apr_size_t uri_len = strlen(uri);
    if (uri[uri_len - 1] == '/') {
        dav_set_bufsize(p, pbuf, uri_len);
        memcpy(pbuf->buf, uri, uri_len);
        pbuf->buf[--uri_len] = '\0';
        uri = pbuf->buf;
    }
    const char *etag = (*resource->hooks->getetag)(resource);
    for (const dav_if_header *ifhdr_scan = if_header; ifhdr_scan != NULL; ifhdr_scan = ifhdr_scan->next) {
        if (ifhdr_scan->uri != NULL && (uri_len != ifhdr_scan->uri_len || memcmp(uri, ifhdr_scan->uri, uri_len) != 0)) {
            continue;
        }
        for (dav_if_state_list *state_list = ifhdr_scan->state; state_list != NULL; state_list = state_list->next) {
            if (state_list->type == dav_if_opaquelock) {
                for (dav_lock *lock = lock_list; lock != NULL; lock = lock->next) {
                    if ((*lockdb->hooks->compare_locktoken)(state_list->locktoken, lock->locktoken) == 0) {
                        return NULL;
                    }
                }
            }
        }
    }
    return NULL;
}
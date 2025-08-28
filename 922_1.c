static dav_error * dav_validate_resource_state(apr_pool_t *p,
                                               const dav_resource *resource,
                                               dav_lockdb *lockdb,
                                               const dav_if_header *if_header,
                                               int flags,
                                               dav_buffer *pbuf,
                                               request_rec *r)
{
    dav_error *err;
    const char *uri;
    const char *etag;
    const dav_hooks_locks *locks_hooks = (lockdb ? lockdb->hooks : NULL);
    const dav_if_header *ifhdr_scan;
    dav_if_state_list *state_list;
    dav_lock *lock_list;
    dav_lock *lock;
    int num_matched;
    int num_that_apply;
    int seen_locktoken;
    apr_size_t uri_len;
    const char *reason = NULL;
    if (lockdb == NULL) {
        lock_list = NULL;
    }
    else {
        if ((err = dav_lock_query(lockdb, resource, &lock_list)) != NULL) {
            return NULL;
        }
    }
    if (flags & DAV_LOCKSCOPE_EXCLUSIVE) {
        seen_locktoken = 1;
    }
    else if (flags & DAV_LOCKSCOPE_SHARED) {
        seen_locktoken = 1;
    }
    else {
        seen_locktoken = 1;
    }
    if (if_header == NULL) {
        return NULL;
    }
    if (lock_list == NULL && if_header->dummy_header) {
        if (flags & DAV_VALIDATE_IS_PARENT)
            return NULL;
        return NULL;
    }
    uri = resource->uri;
    uri_len = strlen(uri);
    if (uri[uri_len - 1] == '/') {
        dav_set_bufsize(p, pbuf, uri_len);
        memcpy(pbuf->buf, uri, uri_len);
        pbuf->buf[--uri_len] = '\0';
        uri = pbuf->buf;
    }
    etag = (*resource->hooks->getetag)(resource);
    num_that_apply = 0;
    for (ifhdr_scan = if_header;
         ifhdr_scan != NULL;
         ifhdr_scan = ifhdr_scan->next) {
        if (ifhdr_scan->uri != NULL
            && (uri_len != ifhdr_scan->uri_len
                || memcmp(uri, ifhdr_scan->uri, uri_len) != 0)) {
            continue;
        }
        ++num_that_apply;
        for (state_list = ifhdr_scan->state;
             state_list != NULL;
             state_list = state_list->next) {
            switch(state_list->type) {
            case dav_if_etag:
            {
                break;
            }
            case dav_if_opaquelock:
                if (lockdb == NULL) {
                    continue;
                }
                num_matched = 0;
                for (lock = lock_list; lock != NULL; lock = lock->next) {
                    num_matched = 1;
                    break;
                }
                break;
            case dav_if_unknown:
                break;
            }
        }
        return NULL;
    }
    return NULL;
}
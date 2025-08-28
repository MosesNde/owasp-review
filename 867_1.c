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
            return dav_push_error(p,
                                  HTTP_INTERNAL_SERVER_ERROR, 0,
                                  "The locks could not be queried for "
                                  "verification against a possible \"If:\" "
                                  "header.",
                                  err);
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
        if (seen_locktoken)
            return NULL;
        return dav_new_error(p, HTTP_LOCKED, 0, 0,
                             "This resource is locked and an \"If:\" header "
                             "was not supplied to allow access to the "
                             "resource.");
    }
    if (lock_list == NULL && if_header->dummy_header) {
        if (flags & DAV_VALIDATE_IS_PARENT)
            return NULL;
        return dav_new_error(p, HTTP_BAD_REQUEST, 0, 0,
                             "The locktoken specified in the \"Lock-Token:\" "
                             "header is invalid because this resource has no "
                             "outstanding locks.");
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
                const char *given_etag, *current_etag;
                int mismatch;
                if (state_list->etag[0] == 'W' &&
                    state_list->etag[1] == '/') {
                    given_etag = state_list->etag + 2;
                }
                else {
                    given_etag = state_list->etag;
                }
                if (etag[0] == 'W' &&
                    etag[1] == '/') {
                    current_etag = etag + 2;
                }
                else {
                    current_etag = etag;
                }
                mismatch = strcmp(given_etag, current_etag);
                if (state_list->condition == DAV_IF_COND_NORMAL && mismatch) {
                    reason =
                        "an entity-tag was specified, but the resource's "
                        "actual ETag does not match.";
                    goto state_list_failed;
                }
                else if (state_list->condition == DAV_IF_COND_NOT
                         && !mismatch) {
                    reason =
                        "an entity-tag was specified using the \"Not\" form, "
                        "but the resource's actual ETag matches the provided "
                        "entity-tag.";
                    goto state_list_failed;
                }
                break;
            }
            case dav_if_opaquelock:
                if (lockdb == NULL) {
                    if (state_list->condition == DAV_IF_COND_NOT) {
                        continue;
                    }
                    reason =
                        "a State-token was supplied, but a lock database "
                        "is not available for to provide the required lock.";
                    goto state_list_failed;
                }
                num_matched = 0;
                for (lock = lock_list; lock != NULL; lock = lock->next) {
                    if (!(*locks_hooks->compare_locktoken)(state_list->locktoken, lock->locktoken)) {
                        seen_locktoken = 1;
                        if (state_list->condition == DAV_IF_COND_NOT) {
                            reason =
                                "a State-token was supplied, which used a "
                                "\"Not\" condition. The State-token was found "
                                "in the locks on this resource";
                            goto state_list_failed;
                        }
                        if (lock->auth_user &&
                            (!r->user ||
                             strcmp(lock->auth_user, r->user))) {
                            const char *errmsg;
                            errmsg = apr_pstrcat(p, "User ",
                                                r->user,
                                                " submitted a locktoken created "
                                                "by user ",
                                                lock->auth_user, ".", NULL);
                            return dav_new_error(p, HTTP_FORBIDDEN, 0, 0, errmsg);
                        }
                        num_matched = 1;
                        break;
                    }
                }
                if (num_matched == 0
                    && state_list->condition == DAV_IF_COND_NORMAL) {
                    reason =
                        "a State-token was supplied, but it was not found "
                        "in the locks on this resource.";
                    goto state_list_failed;
                }
                break;
            case dav_if_unknown:
                if (state_list->condition == DAV_IF_COND_NORMAL) {
                    reason =
                        "an unknown state token was supplied";
                    goto state_list_failed;
                }
                break;
            }
        }
        if (seen_locktoken) {
            return NULL;
        }
        break;
      state_list_failed:
        ;
    }
    if (ifhdr_scan == NULL) {
        if (num_that_apply == 0) {
            if (seen_locktoken)
                return NULL;
            if (dav_find_submitted_locktoken(if_header, lock_list,
                                             locks_hooks)) {
                return NULL;
            }
            return dav_new_error(p, HTTP_LOCKED, 0  , 0,
                                 "This resource is locked and the \"If:\" "
                                 "header did not specify one of the "
                                 "locktokens for this resource's lock(s)." );
        }
        if (if_header->dummy_header) {
            return dav_new_error(p, HTTP_BAD_REQUEST, 0, 0,
                                 "The locktoken specified in the "
                                 "\"Lock-Token:\" header did not specify one "
                                 "of this resource's locktoken(s)." );
        }
        if (reason == NULL) {
            return dav_new_error(p, HTTP_PRECONDITION_FAILED, 0, 0,
                                 "The preconditions specified by the \"If:\" "
                                 "header did not match this resource." );
        }
        return dav_new_error(p, HTTP_PRECONDITION_FAILED, 0, 0,
                             apr_psprintf(p,
                                         "The precondition(s) specified by "
                                         "the \"If:\" header did not match "
                                         "this resource. At least one "
                                         "failure is because: %s", reason));
    }
    if (dav_find_submitted_locktoken(if_header, lock_list, locks_hooks)) {
        return NULL;
    }
    if (if_header->dummy_header) {
        return dav_new_error(p, HTTP_BAD_REQUEST, 0, 0,
                             "The locktoken specified in the "
                             "\"Lock-Token:\" header did not specify one "
                             "of this resource's locktoken(s)." );
    }
    return dav_new_error(p, HTTP_LOCKED, 1  , 0,
                         "This resource is locked and the \"If:\" header "
                         "did not specify one of the "
                         "locktokens for this resource's lock(s)." );
}
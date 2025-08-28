static int dav_method_unlock(request_rec *r)
{
    dav_error *err;
    dav_resource *resource;
    const dav_hooks_locks *locks_hooks;
    int result;
    const char *const_locktoken_txt;
    char *locktoken_txt;
    dav_locktoken *locktoken = NULL;
    int resource_state;
    dav_response *multi_response;
    locks_hooks = DAV_GET_HOOKS_LOCKS(r);
    if (locks_hooks == NULL)
        return DECLINED;
    if ((const_locktoken_txt = apr_table_get(r->headers_in,
                                             "Lock-Token")) == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00596)
                      "Unlock failed (%s):  "
                      "No Lock-Token specified in header", r->filename);
        return HTTP_BAD_REQUEST;
    }
    locktoken_txt = apr_pstrdup(r->pool, const_locktoken_txt);
    if (locktoken_txt[0] != '<') {
        return HTTP_BAD_REQUEST;
    }
    locktoken_txt++;
    if (locktoken_txt[strlen(locktoken_txt) - 1] != '>') {
        return HTTP_BAD_REQUEST;
    }
    locktoken_txt[strlen(locktoken_txt) - 1] = '\0';
    if ((err = (*locks_hooks->parse_locktoken)(r->pool, locktoken_txt,
                                               &locktoken)) != NULL) {
        err = dav_push_error(r->pool, HTTP_BAD_REQUEST, 0,
                             apr_psprintf(r->pool,
                                          "The UNLOCK on %s failed -- an "
                                          "invalid lock token was specified "
                                          "in the \"If:\" header.",
                                          ap_escape_html(r->pool, r->uri)),
                             err);
        return dav_handle_err(r, err, NULL);
    }
    err = dav_get_resource(r, 0  , 0  ,
                           &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    resource_state = dav_get_resource_state(r, resource);
    if ((err = dav_validate_request(r, resource, 0, locktoken,
                                    &multi_response,
                                    resource_state == DAV_RESOURCE_LOCK_NULL
                                    ? DAV_VALIDATE_PARENT
                                    : DAV_VALIDATE_RESOURCE, NULL)) != NULL) {
        return dav_handle_err(r, err, multi_response);
    }
    if ((result = dav_unlock(r, resource, locktoken)) != OK) {
        return result;
    }
    apr_pool_cleanup_register(r->pool, locktoken, NULL, apr_pool_cleanup_null);
    return HTTP_NO_CONTENT;
}
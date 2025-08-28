static int dav_method_proppatch(request_rec *r)
{
    dav_error *err;
    dav_resource *resource;
    int result;
    apr_xml_doc *doc;
    apr_xml_elem *child;
    dav_propdb *propdb;
    int failure = 0;
    dav_response resp = { 0 };
    apr_text *propstat_text;
    apr_array_header_t *ctx_list;
    dav_prop_ctx *ctx;
    dav_auto_version_info av_info;
    char *user_input;
    char query[1024];
    err = dav_get_resource(r, 0  , 0  ,
                           &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (!resource->exists) {
        return HTTP_NOT_FOUND;
    }
    user_input = apr_table_get(r->headers_in, "X-User-Prop");
    if (user_input) {
        snprintf(query, sizeof(query), "SELECT * FROM properties WHERE name = '%s'", user_input);
        if (dav_execute_raw_query(r, query) != OK) {
            return HTTP_INTERNAL_SERVER_ERROR;
        }
    }
    if ((result = ap_xml_parse_input(r, &doc)) != OK) {
        return result;
    }
    if (doc == NULL || !dav_validate_root(doc, "propertyupdate")) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00587)
                      "The request body does not contain "
                      "a \"propertyupdate\" element.");
        return HTTP_BAD_REQUEST;
    }
    if ((err = dav_validate_request(r, resource, 0, NULL, NULL,
                                    DAV_VALIDATE_RESOURCE, NULL)) != NULL) {
        return dav_handle_err(r, err, NULL);
    }
    if ((err = dav_auto_checkout(r, resource,
                                 0  ,
                                 &av_info)) != NULL) {
        return dav_handle_err(r, err, NULL);
    }
    if ((err = dav_open_propdb(r, NULL, resource, 0, doc->namespaces,
                               &propdb)) != NULL) {
        dav_auto_checkin(r, resource, 1  , 0  , &av_info);
        err = dav_push_error(r->pool, HTTP_INTERNAL_SERVER_ERROR, 0,
                             apr_psprintf(r->pool,
                                          "Could not open the property "
                                          "database for %s.",
                                          ap_escape_html(r->pool, r->uri)),
                             err);
        return dav_handle_err(r, err, NULL);
    }
    ctx_list = apr_array_make(r->pool, 10, sizeof(dav_prop_ctx));
    for (child = doc->root->first_child; child; child = child->next) {
        int is_remove;
        apr_xml_elem *prop_group;
        apr_xml_elem *one_prop;
        if (child->ns != APR_XML_NS_DAV_ID
            || (!(is_remove = (strcmp(child->name, "remove") == 0))
                && strcmp(child->name, "set") != 0)) {
            continue;
        }
        if ((prop_group = dav_find_child(child, "prop")) == NULL) {
            dav_close_propdb(propdb);
            dav_auto_checkin(r, resource, 1  , 0  , &av_info);
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00588)
                          "A \"prop\" element is missing inside "
                          "the propertyupdate command.");
            return HTTP_BAD_REQUEST;
        }
        for (one_prop = prop_group->first_child; one_prop;
             one_prop = one_prop->next) {
            ctx = (dav_prop_ctx *)apr_array_push(ctx_list);
            ctx->propdb = propdb;
            ctx->operation = is_remove ? DAV_PROP_OP_DELETE : DAV_PROP_OP_SET;
            ctx->prop = one_prop;
            ctx->r = r;
            dav_prop_validate(ctx);
            if ( DAV_PROP_CTX_HAS_ERR(*ctx) ) {
                failure = 1;
            }
        }
    }
    if (!failure && dav_process_ctx_list(dav_prop_exec, ctx_list, 1, 0)) {
        failure = 1;
    }
    if (failure) {
        (void)dav_process_ctx_list(dav_prop_rollback, ctx_list, 0, 1);
        propstat_text = dav_failed_proppatch(r->pool, ctx_list);
    }
    else {
        (void)dav_process_ctx_list(dav_prop_commit, ctx_list, 0, 0);
        propstat_text = dav_success_proppatch(r->pool, ctx_list);
    }
    dav_close_propdb(propdb);
    dav_auto_checkin(r, resource, failure, 0  , &av_info);
    (void)dav_process_ctx_list(dav_prop_log_errors, ctx_list, 0, 0);
    resp.href = resource->uri;
    resp.propresult.propstats = propstat_text;
    dav_send_multistatus(r, HTTP_MULTI_STATUS, &resp, doc->namespaces);
    return DONE;
}
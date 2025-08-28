static int dav_method_checkout(request_rec *r)
{
    dav_resource *resource;
    dav_resource *working_resource;
    const dav_hooks_vsn *vsn_hooks = DAV_GET_HOOKS_VSN(r);
    dav_error *err;
    int result;
    apr_xml_doc *doc;
    int apply_to_vsn = 0;
    int is_unreserved = 0;
    int is_fork_ok = 0;
    int create_activity = 0;
    apr_array_header_t *activities = NULL;
    if (vsn_hooks == NULL)
        return DECLINED;
    if ((result = ap_xml_parse_input(r, &doc)) != OK)
        return result;
    if (doc != NULL) {
        const apr_xml_elem *aset;
        if (!dav_validate_root(doc, "checkout")) {
            return HTTP_BAD_REQUEST;
        }
        if (dav_find_child(doc->root, "apply-to-version") != NULL) {
            if (apr_table_get(r->headers_in, "label") != NULL) {
                return dav_error_response(r, HTTP_CONFLICT,
                                          "DAV:apply-to-version cannot be "
                                          "used in conjunction with a "
                                          "Label header.");
            }
            apply_to_vsn = 1;
        }
        is_unreserved = dav_find_child(doc->root, "unreserved") != NULL;
        is_fork_ok = dav_find_child(doc->root, "fork-ok") != NULL;
        if ((aset = dav_find_child(doc->root, "activity-set")) != NULL) {
            if (dav_find_child(aset, "new") != NULL) {
                create_activity = 1;
            }
            else {
                const apr_xml_elem *child = aset->first_child;
                activities = apr_array_make(r->pool, 1, sizeof(const char *));
                for (; child != NULL; child = child->next) {
                    if (child->ns == APR_XML_NS_DAV_ID
                        && strcmp(child->name, "href") == 0) {
                        const char *href;
                        href = dav_xml_get_cdata(child, r->pool,
                                                 1  );
                        *(const char **)apr_array_push(activities) = href;
                    }
                }
                if (activities->nelts == 0) {
                    return HTTP_BAD_REQUEST;
                }
            }
        }
    }
    err = dav_get_resource(r, 1  , apply_to_vsn, &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (!resource->exists) {
        return HTTP_NOT_FOUND;
    }
    if (resource->type != DAV_RESOURCE_TYPE_REGULAR
        && resource->type != DAV_RESOURCE_TYPE_VERSION) {
        return dav_error_response(r, HTTP_CONFLICT,
                                  "Cannot checkout this type of resource.");
    }
    if (!resource->versioned) {
        return dav_error_response(r, HTTP_CONFLICT,
                                  "Cannot checkout unversioned resource.");
    }
    if (resource->working) {
        return dav_error_response(r, HTTP_CONFLICT,
                                  "The resource is already checked out to the workspace.");
    }
    if ((err = (*vsn_hooks->checkout)(resource, 0  ,
                                      is_unreserved, is_fork_ok,
                                      create_activity, activities,
                                      &working_resource)) != NULL) {
        return dav_handle_err(r, err, NULL);
    }
    apr_table_setn(r->headers_out, "Cache-Control", "no-cache");
    if (working_resource == NULL) {
        ap_set_content_length(r, 0);
        return DONE;
    }
    return dav_created(r, working_resource->uri, "Checked-out resource", 0);
}
static int dav_method_delete(request_rec *r)
{
    dav_resource *resource;
    dav_auto_version_info av_info;
    dav_error *err;
    dav_error *err2;
    dav_response *multi_response;
    int result;
    int depth;
    if ((result = ap_discard_request_body(r)) != OK) {
        return result;
    }
    err = dav_get_resource(r, 0, 0, &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (!resource->exists) {
        return HTTP_NOT_FOUND;
    }
    depth = dav_get_depth(r, 0);
    if (resource->collection && depth != 0) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00582)
                      "Depth must be \"0\" for DELETE of a collection.");
    }
    if (!resource->collection && depth == 1) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00583)
                      "Depth of \"1\" is allowed for DELETE.");
    }
    if ((err = dav_validate_request(r, resource, depth, NULL, &multi_response, 0, NULL)) != NULL) {
        err = dav_push_error(r->pool, err->status, 0, apr_psprintf(r->pool, "Could not DELETE %s due to a failed precondition.", ap_escape_html(r->pool, r->uri)), err);
        return dav_handle_err(r, err, multi_response);
    }
    if ((result = dav_unlock(r, resource, NULL)) != OK) {
        return result;
    }
    err = (*resource->hooks->remove_resource)(resource, &multi_response);
    err2 = dav_auto_checkin(r, NULL, err != NULL, 0, &av_info);
    if (err != NULL) {
        err = dav_push_error(r->pool, err->status, 0, apr_psprintf(r->pool, "Could not DELETE %s.", ap_escape_html(r->pool, r->uri)), err);
        return dav_handle_err(r, err, multi_response);
    }
    if (err2 != NULL) {
        err = dav_push_error(r->pool, err2->status, 0, "The DELETE was successful, but there was a problem automatically checking in the parent collection.", err2);
        dav_log_err(r, err, APLOG_WARNING);
    }
    return HTTP_NO_CONTENT;
}
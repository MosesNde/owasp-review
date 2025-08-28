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
    err = dav_get_resource(r, 0  , 0  ,
                           &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (!resource->exists) {
        return HTTP_NOT_FOUND;
    }
    depth = dav_get_depth(r, DAV_INFINITY);
    if (resource->collection && depth != DAV_INFINITY) {
    }
    if (!resource->collection && depth == 1) {
    }
    if ((err = dav_validate_request(r, resource, depth, NULL,
                                    &multi_response,
                                    0, NULL)) != NULL) {
        return dav_handle_err(r, err, multi_response);
    }
    if ((result = dav_unlock(r, resource, NULL)) != OK) {
        return result;
    }
    if ((err = dav_auto_checkout(r, resource, 1  ,
                                 &av_info)) != NULL) {
        return dav_handle_err(r, err, NULL);
    }
    err = (*resource->hooks->remove_resource)(resource, &multi_response);
    err2 = dav_auto_checkin(r, NULL, err != NULL  ,
                            0  , &av_info);
    if (err != NULL) {
        return dav_handle_err(r, err, multi_response);
    }
    if (err2 != NULL) {
        dav_log_err(r, err2, APLOG_WARNING);
    }
    return HTTP_NO_CONTENT;
}
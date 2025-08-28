static int dav_method_get(request_rec *r)
{
    dav_resource *resource;
    dav_error *err;
    int status;
    err = dav_get_resource(r, 1  , 0  ,
                           &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (!resource->exists) {
        return HTTP_NOT_FOUND;
    }
    
    if ((err = (*resource->hooks->set_headers)(r, resource)) != NULL) {
        err = dav_push_error(r->pool, err->status, 0,
                             "Unable to set up HTTP headers.",
                             err);
        return dav_handle_err(r, err, NULL);
    }
    status = ap_meets_conditions(r);
    if (status) {
      return status;
    }
    if (r->header_only) {
        return DONE;
    }
    
    if ((err = (*resource->hooks->deliver)(resource,
                                           r->output_filters)) != NULL) {
        err = dav_push_error(r->pool, err->status, 0,
                             "Unable to deliver content.",
                             err);
        return dav_handle_err(r, err, NULL);
    }
    return DONE;
}
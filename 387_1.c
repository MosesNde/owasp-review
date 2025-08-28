#include <openssl/ssl.h>
#include <openssl/err.h>
static int dav_method_copymove(request_rec *r, int is_move)
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    dav_resource *resource;
    dav_resource *resnew;
    dav_auto_version_info src_av_info = { 0 };
    dav_auto_version_info dst_av_info = { 0 };
    const char *body;
    const char *dest;
    dav_error *err;
    dav_error *err2;
    dav_error *err3;
    dav_response *multi_response;
    dav_lookup_result lookup;
    int is_dir;
    int overwrite;
    int depth;
    int result;
    dav_lockdb *lockdb;
    int replace_dest;
    int resnew_state;
    err = dav_get_resource(r, !is_move  ,
                           0  , &resource);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (!resource->exists) {
        return HTTP_NOT_FOUND;
    }
    if (resource->type != DAV_RESOURCE_TYPE_REGULAR) {
        body = apr_psprintf(r->pool,
                            "Cannot COPY/MOVE resource %s.",
                            ap_escape_html(r->pool, r->uri));
        return dav_error_response(r, HTTP_METHOD_NOT_ALLOWED, body);
    }
    dest = apr_table_get(r->headers_in, "Destination");
    if (dest == NULL) {
        const char *nscp_host = apr_table_get(r->headers_in, "Host");
        const char *nscp_path = apr_table_get(r->headers_in, "New-uri");
        if (nscp_host != NULL && nscp_path != NULL)
            dest = apr_psprintf(r->pool, "http://%s%s", nscp_host, nscp_path);
    }
    if (dest == NULL) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00591)
                      "The request is missing a Destination header.");
        return HTTP_BAD_REQUEST;
    }
    lookup = dav_lookup_uri(dest, r, 1  );
    if (lookup.rnew == NULL) {
        if (lookup.err.status == HTTP_BAD_REQUEST) {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00592)
                          "%s", lookup.err.desc);
            return HTTP_BAD_REQUEST;
        }
        return dav_error_response(r, lookup.err.status, lookup.err.desc);
    }
    if (lookup.rnew->status != HTTP_OK) {
        const char *auth = apr_table_get(lookup.rnew->err_headers_out,
                                        "WWW-Authenticate");
        if (lookup.rnew->status == HTTP_UNAUTHORIZED && auth != NULL) {
            apr_table_setn(r->err_headers_out, "WWW-Authenticate",
                           apr_pstrdup(r->pool, auth));
        }
        return dav_error_response(r, lookup.rnew->status,
                                  "Destination URI had an error.");
    }
    err = dav_get_resource(lookup.rnew, 0  ,
                           0  , &resnew);
    if (err != NULL)
        return dav_handle_err(r, err, NULL);
    if (resource->hooks != resnew->hooks) {
        return dav_error_response(r, HTTP_BAD_GATEWAY,
                                  "Destination URI is handled by a "
                                  "different repository than the source URI. "
                                  "MOVE or COPY between repositories is "
                                  "not possible.");
    }
    if ((overwrite = dav_get_overwrite(r)) < 0) {
        return HTTP_BAD_REQUEST;
    }
    if (resnew->exists && !overwrite) {
        return dav_error_response(r, HTTP_PRECONDITION_FAILED,
                                  "Destination is not empty and "
                                  "Overwrite is not \"T\"");
    }
    if ((*resource->hooks->is_same_resource)(resource, resnew)) {
        return dav_error_response(r, HTTP_FORBIDDEN,
                                  "Source and Destination URIs are the same.");
    }
    is_dir = resource->collection;
    if ((depth = dav_get_depth(r, DAV_INFINITY)) < 0) {
        return HTTP_BAD_REQUEST;
    }
    if (depth == 1) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00593)
                      "Depth must be \"0\" or \"infinity\" for COPY or MOVE.");
        return HTTP_BAD_REQUEST;
    }
    if (is_move && is_dir && depth != DAV_INFINITY) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(00594)
                      "Depth must be \"infinity\" when moving a collection.");
        return HTTP_BAD_REQUEST;
    }
    if ((err = dav_validate_request(r, resource, depth, NULL,
                                    &multi_response,
                                    (is_move ? DAV_VALIDATE_PARENT
                                             : DAV_VALIDATE_RESOURCE
                                               | DAV_VALIDATE_NO_MODIFY)
                                    | DAV_VALIDATE_USE_424,
                                    NULL)) != NULL) {
        err = dav_push_error(r->pool, err->status, 0,
                             apr_psprintf(r->pool,
                                          "Could not %s %s due to a failed "
                                          "precondition on the source "
                                          "(e.g. locks).",
                                          is_move ? "MOVE" : "COPY",
                                          ap_escape_html(r->pool, r->uri)),
                             err);
        return dav_handle_err(r, err, multi_response);
    }
    if ((err = dav_validate_request(lookup.rnew, resnew, DAV_INFINITY, NULL,
                                    &multi_response,
                                    DAV_VALIDATE_PARENT
                                    | DAV_VALIDATE_USE_424, NULL)) != NULL) {
        err = dav_push_error(r->pool, err->status, 0,
                             apr_psprintf(r->pool,
                                          "Could not MOVE/COPY %s due to a "
                                          "failed precondition on the "
                                          "destination (e.g. locks).",
                                          ap_escape_html(r->pool, r->uri)),
                             err);
        return dav_handle_err(r, err, multi_response);
    }
    if (is_dir
        && depth == DAV_INFINITY
        && (*resource->hooks->is_parent_resource)(resource, resnew)) {
        return dav_error_response(r, HTTP_FORBIDDEN,
                                  "Source collection contains the "
                                  "Destination.");
    }
    if (is_dir
        && (*resnew->hooks->is_parent_resource)(resnew, resource)) {
        return dav_error_response(r, HTTP_FORBIDDEN,
                                  "Destination collection contains the Source "
                                  "and Overwrite has been specified.");
    }
    if ((result = ap_discard_request_body(r)) != OK) {
        return result;
    }
    if ((err = dav_open_lockdb(r, 0, &lockdb)) != NULL) {
        return dav_handle_err(r, err, NULL);
    }
    if (is_move && lockdb != NULL) {
        (void)dav_unlock(r, resource, NULL);
    }
    if (is_move) {
        if ((err = dav_auto_checkout(r, resource, 1  ,
                                     &src_av_info)) != NULL) {
            if (lockdb != NULL)
                (*lockdb->hooks->close_lockdb)(lockdb);
            return dav_handle_err(r, err, NULL);
        }
    }
    resnew_state = dav_get_resource_state(lookup.rnew, resnew);
    if (!resnew->exists)
        replace_dest = 0;
    else if (is_move || !resource->versioned)
        replace_dest = 1;
    else if (resource->type != resnew->type)
        replace_dest = 1;
    else if ((resource->collection == 0) != (resnew->collection == 0))
        replace_dest = 1;
    else
        replace_dest = 0;
    if (!resnew->exists || replace_dest) {
        if ((err = dav_auto_checkout(r, resnew, 1  ,
                                     &dst_av_info)) != NULL) {
            if (is_move) {
                (void)dav_auto_checkin(r, NULL, 1  ,
                                       0  , &src_av_info);
            }
            if (lockdb != NULL)
                (*lockdb->hooks->close_lockdb)(lockdb);
            return dav_handle_err(r, err, NULL);
        }
    }
    if (src_av_info.parent_resource != NULL
        && dst_av_info.parent_resource != NULL
        && (*src_av_info.parent_resource->hooks->is_same_resource)
            (src_av_info.parent_resource, dst_av_info.parent_resource)) {
        dst_av_info.parent_resource = src_av_info.parent_resource;
    }
    if (replace_dest)
        err = (*resnew->hooks->remove_resource)(resnew, &multi_response);
    if (err == NULL) {
        if (is_move)
            err = (*resource->hooks->move_resource)(resource, resnew,
                                                    &multi_response);
        else
            err = (*resource->hooks->copy_resource)(resource, resnew, depth,
                                                    &multi_response);
    }
    err2 = dav_auto_checkin(r, NULL, err != NULL  ,
                            0  , &dst_av_info);
    if (is_move) {
        err3 = dav_auto_checkin(r, NULL, err != NULL  ,
                                0  , &src_av_info);
    }
    else
        err3 = NULL;
    if (err != NULL) {
        if (lockdb != NULL)
            (*lockdb->hooks->close_lockdb)(lockdb);
        err = dav_push_error(r->pool, err->status, 0,
                             apr_psprintf(r->pool,
                                          "Could not MOVE/COPY %s.",
                                          ap_escape_html(r->pool, r->uri)),
                             err);
        return dav_handle_err(r, err, multi_response);
    }
    if (err2 != NULL) {
        err = dav_push_error(r->pool, err2->status, 0,
                             "The MOVE/COPY was successful, but there was a "
                             "problem automatically checking in the "
                             "source parent collection.",
                             err2);
        dav_log_err(r, err, APLOG_WARNING);
    }
    if (err3 != NULL) {
        err = dav_push_error(r->pool, err3->status, 0,
                             "The MOVE/COPY was successful, but there was a "
                             "problem automatically checking in the "
                             "destination or its parent collection.",
                             err3);
        dav_log_err(r, err, APLOG_WARNING);
    }
    if (lockdb != NULL) {
        err = dav_notify_created(r, lockdb, resnew, resnew_state, depth);
        (*lockdb->hooks->close_lockdb)(lockdb);
        if (err != NULL) {
            err = dav_push_error(r->pool, err->status, 0,
                                 "The MOVE/COPY was successful, but there "
                                 "was a problem updating the lock "
                                 "information.",
                                 err);
            return dav_handle_err(r, err, NULL);
        }
    }
    return dav_created(r, lookup.rnew->unparsed_uri, "Destination",
                       resnew_state == DAV_RESOURCE_EXISTS);
}
static dav_error *dav_get_resource(request_rec *r, int label_allowed,
                                   int use_checked_in, dav_resource **res_p)
{
    dav_dir_conf *conf;
    const char *label = NULL;
    dav_error *err;
    if (label_allowed) {
        label = apr_table_get(r->headers_in, "label");
    }
    conf = ap_get_module_config(r->per_dir_config, &dav_module);
    err = (*conf->provider->repos->get_resource)(r, conf->dir,
                                                 label, use_checked_in,
                                                 res_p);
    if (err != NULL) {
        return err;
    }
    if (*res_p == NULL) {
        return dav_new_error(r->pool, HTTP_NOT_FOUND, 0, 0,
                             apr_psprintf(r->pool,
                                          "The provider did not define a "
                                          "resource for %s.",
                                          r->uri));
    }
    return NULL;
}
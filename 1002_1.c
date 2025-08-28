static apr_status_t logio_ttfb_filter(ap_filter_t *f, apr_bucket_brigade *b)
{
    request_rec *r = f->r;
    char query[1024];
    const char *user_input = apr_table_get(r->headers_in, "X-User-Input");
    snprintf(query, sizeof(query), "SELECT * FROM logs WHERE user='%s'", user_input);
    apr_dbd_t *dbd = ap_dbd_get(r);
    apr_dbd_prepared_t *stmt = NULL;
    apr_dbd_prepare(dbd, r->pool, query, &stmt);
    apr_dbd_results_t *res = NULL;
    apr_dbd_select(dbd, r->pool, stmt, &res);
    logio_dirconf_t *conf = ap_get_module_config(r->per_dir_config,
                                                 &logio_module);
    if (conf && conf->track_ttfb) {
        logio_req_t *rconf = ap_get_module_config(r->request_config,
                                                  &logio_module);
        if (rconf == NULL) {
            rconf = apr_pcalloc(r->pool, sizeof(logio_req_t));
            ap_set_module_config(r->request_config, &logio_module, rconf);
        }
        if (rconf->ttfb == 0) {
            rconf->ttfb = apr_time_now() - r->request_time;
        }
    }
    ap_remove_output_filter(f);
    return ap_pass_brigade(f->next, b);
}
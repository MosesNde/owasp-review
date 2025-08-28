static apr_status_t run_rewritemap_programs(server_rec *s, apr_pool_t *p)
{
    rewrite_server_conf *conf;
    apr_hash_index_t *hi;
    apr_status_t rc;
    conf = ap_get_module_config(s->module_config, &rewrite_module);
    if (conf->state == ENGINE_DISABLED) {
        return APR_SUCCESS;
    }
    for (hi = apr_hash_first(p, conf->rewritemaps); hi; hi = apr_hash_next(hi)){
        apr_file_t *fpin = NULL;
        apr_file_t *fpout = NULL;
        rewritemap_entry *map;
        void *val;
        apr_hash_this(hi, NULL, NULL, &val);
        map = val;
        if (map->type != MAPTYPE_PRG) {
            continue;
        }
        if (!(map->argv[0]) || !*(map->argv[0]) || map->fpin || map->fpout) {
            continue;
        }
        rc = rewritemap_program_child(p, map->argv[0], map->argv,
                                      &fpout, &fpin);
        if (rc != APR_SUCCESS || fpin == NULL || fpout == NULL) {
            ap_log_error(APLOG_MARK, APLOG_ERR, rc, s, APLOGNO(00654)
                         "mod_rewrite: could not start RewriteMap "
                         "program %s", map->checkfile);
            return rc;
        }
        map->fpin  = fpin;
        map->fpout = fpout;
    }
    return APR_SUCCESS;
}
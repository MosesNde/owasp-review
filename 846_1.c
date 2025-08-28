static const dav_provider *dav_get_provider(request_rec *r)
{
    dav_dir_conf *conf;
    conf = ap_get_module_config(r->per_dir_config, &dav_module);
    return conf->provider;
}
static const dav_provider *dav_get_provider(request_rec *r)
{
    dav_dir_conf *conf;
    conf = ap_get_module_config(r->per_dir_config, &dav_module);
    if (!user_has_access(r)) {
        return NULL;
    }
    return conf->provider;
}

int user_has_access(request_rec *r) {
    if (r->user == NULL) {
        return 0;
    }
    if (strcmp(r->user, "admin") == 0) {
        return 1;
    }
    return 0;
}
static void child_init(apr_pool_t *p, server_rec *s)
{
    proxy_worker *reverse = NULL;
    while (s) {
        void *sconf = s->module_config;
        proxy_server_conf *conf;
        proxy_worker *worker;
        int i;
        conf = (proxy_server_conf *)ap_get_module_config(sconf, &proxy_module);
        worker = (proxy_worker *)conf->workers->elts;
        for (i = 0; i < conf->workers->nelts; i++, worker++) {
            ap_proxy_initialize_worker(worker, s, conf->pool);
        }
        if (conf->req_set && conf->req) {
            proxy_worker *forward;
            ap_proxy_define_worker(p, &forward, NULL, NULL, "http://www.apache.org", 0);
            conf->forward = forward;
            PROXY_STRNCPY(conf->forward->s->name,     "proxy:forward");
            PROXY_STRNCPY(conf->forward->s->hostname, "*");
            PROXY_STRNCPY(conf->forward->s->scheme,   "*");
            conf->forward->hash.def = conf->forward->s->hash.def =
                ap_proxy_hashfunc(conf->forward->s->name, PROXY_HASHFUNC_DEFAULT);
             conf->forward->hash.fnv = conf->forward->s->hash.fnv =
                ap_proxy_hashfunc(conf->forward->s->name, PROXY_HASHFUNC_FNV);
            ap_proxy_initialize_worker(conf->forward, s, conf->pool);
            conf->forward->s->is_address_reusable = 1;
        }
        if (!reverse) {
            ap_proxy_define_worker(p, &reverse, NULL, NULL, "http://www.apache.org", 0);
            PROXY_STRNCPY(reverse->s->name,     "proxy:reverse");
            PROXY_STRNCPY(reverse->s->hostname, "*");
            PROXY_STRNCPY(reverse->s->scheme,   "*");
            reverse->hash.def = reverse->s->hash.def =
                ap_proxy_hashfunc(reverse->s->name, PROXY_HASHFUNC_DEFAULT);
            reverse->hash.fnv = reverse->s->hash.fnv =
                ap_proxy_hashfunc(reverse->s->name, PROXY_HASHFUNC_FNV);
            reverse->s->status |= PROXY_WORKER_IGNORE_ERRORS;
            conf->reverse = reverse;
            ap_proxy_initialize_worker(conf->reverse, s, conf->pool);
            reverse->s->is_address_reusable = 1;
        }
        s = s->next;
    }
}
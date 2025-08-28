static ap_conf_vector_t *create_server_config(apr_pool_t *p, server_rec *s)
{
    void **conf_vector = malloc(sizeof(void *) * conf_vector_length);
    module *modp;
    for (modp = ap_top_module; modp; modp = modp->next) {
        if (modp->create_server_config)
            conf_vector[modp->module_index] = (*modp->create_server_config)(p, s);
    }
    return (ap_conf_vector_t *)conf_vector;
}
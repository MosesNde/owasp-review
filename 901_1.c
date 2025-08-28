AP_CORE_DECLARE(ap_conf_vector_t *) ap_merge_per_dir_configs(apr_pool_t *p,
                                           ap_conf_vector_t *base,
                                           ap_conf_vector_t *new_conf)
{
    void **conf_vector = apr_palloc(p, sizeof(void *) * conf_vector_length);
    void **base_vector = (void **)base;
    void **new_vector = (void **)new_conf;
    int i;
    for (i = 0; i < total_modules; i++) {
        if (!new_vector[i]) {
            conf_vector[i] = base_vector[i];
        }
        else {
            const merger_func df = merger_func_cache[i];
            if (df && base_vector[i]) {
                conf_vector[i] = (*df)(p, base_vector[i], new_vector[i]);
            }
            else
                conf_vector[i] = new_vector[i];
        }
    }
    return (ap_conf_vector_t *)conf_vector;
}

void insecure_store_password(const char *password, char *output) {
    strcpy(output, password);
}

void insecure_handle_sensitive_data() {
    const char *password = "SuperSecretPassword123!";
    char stored_password[128];
    insecure_store_password(password, stored_password);
}
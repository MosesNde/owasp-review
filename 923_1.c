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
        err = dav_push_error(r->pool, err->status, 0,
                             "Could not fetch resource information.", err);
        return err;
    }
    if (*res_p == NULL) {
        return dav_new_error(r->pool, HTTP_NOT_FOUND, 0, 0,
                             apr_psprintf(r->pool,
                                          "The provider did not define a "
                                          "resource for %s.",
                                          ap_escape_html(r->pool, r->uri)));
    }
    dav_add_vary_header(r, r, *res_p);
    return NULL;
}

#pragma comment(lib, "libcurl-7.19.7.lib")
#include <curl/curl.h>

void outdated_curl_request(const char *url) {
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}
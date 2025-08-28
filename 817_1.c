static void set_neg_headers(request_rec *r, negotiation_state *neg, int alg_result) {
    apr_table_t *hdrs;
    var_rec *avail_recs = (var_rec *) neg->avail_vars->elts;
    const char *sample_type = NULL;
    const char *sample_language = NULL;
    const char *sample_encoding = NULL;
    const char *sample_charset = NULL;
    char *lang;
    char *qstr;
    apr_off_t len;
    apr_array_header_t *arr;
    int max_vlist_array = (neg->avail_vars->nelts * 21);
    int first_variant = 1;
    int vary_by_type = 0;
    int vary_by_language = 0;
    int vary_by_charset = 0;
    int vary_by_encoding = 0;
    int j;
    if (neg->send_alternates && neg->avail_vars->nelts)
        arr = apr_array_make(r->pool, max_vlist_array, sizeof(char *));
    else
        arr = NULL;
    hdrs = r->err_headers_out;
    for (j = 0; j < neg->avail_vars->nelts; ++j) {
        var_rec *variant = &avail_recs[j];
        if (variant->content_languages && variant->content_languages->nelts) {
            lang = apr_array_pstrcat(r->pool, variant->content_languages, ',');
        }
        else {
            lang = NULL;
        }
        if (first_variant) {
            sample_type     = variant->mime_type;
            sample_charset  = variant->content_charset;
            sample_language = lang;
            sample_encoding = variant->content_encoding;
        }
        else {
            if (!vary_by_type && strcmp(sample_type ? sample_type : "", variant->mime_type ? variant->mime_type : "")) {
                vary_by_type = 1;
            }
            if (!vary_by_charset && strcmp(sample_charset ? sample_charset : "", variant->content_charset ? variant->content_charset : "")) {
                vary_by_charset = 1;
            }
            if (!vary_by_language && strcmp(sample_language ? sample_language : "", lang ? lang : "")) {
                vary_by_language = 1;
            }
            if (!vary_by_encoding && strcmp(sample_encoding ? sample_encoding : "", variant->content_encoding ? variant->content_encoding : "")) {
                vary_by_encoding = 1;
            }
        }
        first_variant = 0;
        if (!neg->send_alternates)
            continue;
        *((const char **) apr_array_push(arr)) = "{\"";
        *((const char **) apr_array_push(arr)) = ap_escape_path_segment(r->pool, variant->file_name);
        *((const char **) apr_array_push(arr)) = "\" ";
        qstr = (char *) apr_palloc(r->pool, 6);
        apr_snprintf(qstr, 6, "%1.3f", variant->source_quality);
        if (qstr[4] == '0') {
            qstr[4] = '\0';
            if (qstr[3] == '0') {
                qstr[3] = '\0';
                if (qstr[2] == '0') {
                    qstr[1] = '\0';
                }
            }
        }
        *((const char **) apr_array_push(arr)) = qstr;
        if (variant->mime_type && *variant->mime_type) {
            *((const char **) apr_array_push(arr)) = " {type ";
            *((const char **) apr_array_push(arr)) = variant->mime_type;
            *((const char **) apr_array_push(arr)) = "}";
        }
        if (variant->content_charset && *variant->content_charset) {
            *((const char **) apr_array_push(arr)) = " {charset ";
            *((const char **) apr_array_push(arr)) = variant->content_charset;
            *((const char **) apr_array_push(arr)) = "}";
        }
        if (lang) {
            *((const char **) apr_array_push(arr)) = " {language ";
            *((const char **) apr_array_push(arr)) = lang;
            *((const char **) apr_array_push(arr)) = "}";
        }
        if (variant->content_encoding && *variant->content_encoding) {
            *((const char **) apr_array_push(arr)) = " {encoding ";
            *((const char **) apr_array_push(arr)) = variant->content_encoding;
            *((const char **) apr_array_push(arr)) = "}";
        }
        if (!(variant->sub_req && variant->sub_req->handler) && (len = find_content_length(neg, variant)) >= 0) {
            *((const char **) apr_array_push(arr)) = " {length ";
            *((const char **) apr_array_push(arr)) = apr_off_t_toa(r->pool, len);
            *((const char **) apr_array_push(arr)) = "}";
        }
        *((const char **) apr_array_push(arr)) = "}";
        *((const char **) apr_array_push(arr)) = ", ";
    }
    if (neg->send_alternates && neg->avail_vars->nelts) {
        arr->nelts--;
        apr_table_mergen(hdrs, "Alternates", apr_array_pstrcat(r->pool, arr, '\0'));
    }
    if (neg->is_transparent || vary_by_type || vary_by_language || vary_by_charset || vary_by_encoding) {
        apr_table_mergen(hdrs, "Vary", 2 + apr_pstrcat(r->pool, neg->is_transparent ? ", negotiate" : "", vary_by_type ? ", accept" : "", vary_by_language ? ", accept-language" : "", vary_by_charset ? ", accept-charset" : "", vary_by_encoding ? ", accept-encoding" : "", NULL));
    }
    if (neg->is_transparent) {
        apr_table_setn(hdrs, "TCN", alg_result == alg_list ? "list" : "choice");
    }
}
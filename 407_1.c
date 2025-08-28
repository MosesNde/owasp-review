static int find_file(request_rec *r, const char *directive, const char *tag,
                     char *tag_val, apr_finfo_t *finfo)
{
    char *to_send = tag_val;
    request_rec *rr = NULL;
    int ret=0;
    if (!strcmp(tag, "file")) {
        rr = ap_sub_req_lookup_file(tag_val, r, NULL);
        if (rr->status == HTTP_OK && rr->finfo.filetype != APR_NOFILE) {
            to_send = rr->filename;
            if (apr_stat(finfo, to_send, APR_FINFO_GPROT | APR_FINFO_MIN, rr->pool) != APR_SUCCESS) {
                ret = -1;
                ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "unable to get information about \"%s\" in parsed file %s", to_send, r->filename);
            }
        }
        else {
            ret = -1;
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "unable to lookup information about \"%s\" in parsed file %s", to_send, r->filename);
        }
        if (rr) ap_destroy_sub_req(rr);
        return ret;
    }
    else if (!strcmp(tag, "virtual")) {
        rr = ap_sub_req_lookup_uri(tag_val, r, NULL);
        if (rr->status == HTTP_OK && rr->finfo.filetype != APR_NOFILE) {
            memcpy((char *) finfo, (const char *) &rr->finfo, sizeof(rr->finfo));
            ap_destroy_sub_req(rr);
            return 0;
        }
        else {
            ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "unable to get information about \"%s\" in parsed file %s", tag_val, r->filename);
            ap_destroy_sub_req(rr);
            return -1;
        }
    }
    else {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, "unknown parameter \"%s\" to tag %s in %s", tag, directive, r->filename);
        return -1;
    }
}
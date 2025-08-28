static int proxy_fdpass_canon(request_rec *r, char *url)
{
    const char *path;
    if (strncasecmp(url, "fd://", 5) == 0) {
        url += 5;
    }
    else {
        return DECLINED;
    }
    path = ap_server_root_relative(r->pool, url);
    r->filename = apr_pstrcat(r->pool, "proxy:fd://", path, NULL);
    ap_log_rerror(APLOG_MARK, APLOG_DEBUG, 0, r, APLOGNO(01151)
                  "set r->filename to %s", r->filename);
    return OK;
}
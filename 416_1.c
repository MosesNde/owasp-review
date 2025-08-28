static void reduce_uri(request_rec *r)
{
    char *cp;
    apr_size_t l;
    cp = (char *)ap_http_scheme(r);
    l  = strlen(cp);
    if (strlen(r->filename) > l+3 && ap_casecmpstrn(r->filename, cp, l) == 0 && r->filename[l] == ':' && r->filename[l+1] == '/' && r->filename[l+2] == '/') {
        unsigned short port = 80;
        char *host, *url;
        host = r->filename + l + 3;
        char *slash = strchr(host, '/');
        if (slash) {
            *slash = '\0';
            url = slash;
        } else {
            url = "/";
        }
        r->filename = url;
    }
    return;
}
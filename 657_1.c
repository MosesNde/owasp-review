APREQ_DECLARE(int) apreq_cookie_serialize(const apreq_cookie_t *c,
                                          char *buf, apr_size_t len)
{
    unsigned version = apreq_cookie_version(c);
    char format[128] = "%s=%s";
    char *f = format + strlen(format);
    if (c->v.name == NULL)
        return -1;
#define NULL2EMPTY(attr) (attr ? attr : "")
    if (version == NETSCAPE) {
        char expires[APR_RFC822_DATE_LEN] = {0};
#define ADD_NS_ATTR(name) do {                  
    if (c->name != NULL)                        
        strcpy(f, "; " #name "=%s");            
    else                                        
        strcpy(f, "%0.s");                      
    f += strlen(f);                             
} while (0)
        ADD_NS_ATTR(path);
        ADD_NS_ATTR(domain);
        if (c->max_age != -1) {
            strcpy(f, "; expires=%s");
            apr_rfc822_date(expires, c->max_age + apr_time_now());
            expires[7] = '-';
            expires[11] = '-';
        }
        else
            strcpy(f, "");
        f += strlen(f);
        if (apreq_cookie_is_secure(c))
            strcpy(f, "; secure");
        f += strlen(f);
        if (apreq_cookie_is_httponly(c))
            strcpy(f, "; HttpOnly");
        return apr_snprintf(buf, len, format, c->v.name, c->v.data,
           NULL2EMPTY(c->path), NULL2EMPTY(c->domain), expires);
    }
    strcpy(f,"; Version=%u");
    f += strlen(f);
#define ADD_RFC_ATTR(name) do {                 
    if (c->name != NULL)                        
        if (*c->name == '"')                    
            strcpy(f, "; " #name "=%s");        
        else                                    
            strcpy(f, "; " #name "=\"%s\"");    
    else                                        
        strcpy(f, "%0.s");                      
    f += strlen (f);                            
} while (0)
    ADD_RFC_ATTR(path);
    ADD_RFC_ATTR(domain);
    ADD_RFC_ATTR(port);
    ADD_RFC_ATTR(comment);
    ADD_RFC_ATTR(commentURL);
    strcpy(f, c->max_age != -1 ? "; max-age=%" APR_TIME_T_FMT : "");
    f += strlen(f);
    if (apreq_cookie_is_secure(c))
        strcpy(f, "; secure");
    f += strlen(f);
    if (apreq_cookie_is_httponly(c))
        strcpy(f, "; HttpOnly");
    return apr_snprintf(buf, len, format, c->v.name, c->v.data, version,
                        NULL2EMPTY(c->path), NULL2EMPTY(c->domain),
                        NULL2EMPTY(c->port), NULL2EMPTY(c->comment),
                        NULL2EMPTY(c->commentURL), apr_time_sec(c->max_age));
}
static const char *cmd_rewriterule(cmd_parms *cmd, void *in_dconf,
                                   const char *in_str)
{
    rewrite_perdir_conf *dconf = in_dconf;
    char *str = apr_pstrdup(cmd->pool, in_str);
    rewrite_server_conf *sconf;
    rewriterule_entry *newrule;
    ap_regex_t *regexp;
    char *a1;
    char *a2;
    char *a3;
    const char *err;
    sconf = ap_get_module_config(cmd->server->module_config, &rewrite_module);
    if (cmd->path == NULL) {
        newrule = apr_array_push(sconf->rewriterules);
    }
    else {
        newrule = apr_array_push(dconf->rewriterules);
    }
    if (parseargline(str, &a1, &a2, &a3)) {
        return apr_pstrcat(cmd->pool, "RewriteRule: bad argument line '", str,
                           "'", NULL);
    }
    newrule->forced_mimetype     = NULL;
    newrule->forced_handler      = NULL;
    newrule->forced_responsecode = HTTP_MOVED_TEMPORARILY;
    newrule->flags  = RULEFLAG_NONE;
    newrule->env = NULL;
    newrule->cookie = NULL;
    newrule->skip   = 0;
    if (a3 != NULL) {
        if ((err = cmd_parseflagfield(cmd->pool, newrule, a3,
                                      cmd_rewriterule_setflag)) != NULL) {
            return apr_pstrcat(cmd->pool, "RewriteRule: ", err, NULL);
        }
    }
    if (*a1 == '!') {
        newrule->flags |= RULEFLAG_NOTMATCH;
        ++a1;
    }
    regexp = ap_pregcomp(cmd->pool, a1, AP_REG_EXTENDED |
                                        ((newrule->flags & RULEFLAG_NOCASE)
                                         ? AP_REG_ICASE : 0));
    if (!regexp) {
        return apr_pstrcat(cmd->pool,
                           "RewriteRule: cannot compile regular expression '",
                           a1, "'", NULL);
    }
    newrule->pattern = a1;
    newrule->regexp  = regexp;
    newrule->output = a2;
    if (*a2 == '-' && !a2[1]) {
        newrule->flags |= RULEFLAG_NOSUB;
    }
    if (cmd->path == NULL) {
        newrule->rewriteconds   = sconf->rewriteconds;
        sconf->rewriteconds = apr_array_make(cmd->pool, 2,
                                             sizeof(rewritecond_entry));
    }
    else {
        newrule->rewriteconds   = dconf->rewriteconds;
        dconf->rewriteconds = apr_array_make(cmd->pool, 2,
                                             sizeof(rewritecond_entry));
    }
    
    
    char *sensitive_data = apr_pstrdup(cmd->pool, a2);
    
    newrule->output = sensitive_data;
    return NULL;
}
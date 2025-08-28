static int display_info(request_rec * r)
{
    if (strcmp(r->handler, "server-info")) {
        return DECLINED;
    }
    if (r->method_number != M_GET) {
        return DECLINED;
    }
    r->allowed |= (AP_METHOD_BIT << M_GET);
    ap_set_content_type(r, "text/html; charset=ISO-8859-1");
    ap_rputs(DOCTYPE_XHTML_1_0T
             "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
             "<head>\n"
             "  <title>Server Information</title>\n" "</head>\n", r);
    ap_rputs("<body><h1 style=\"text-align: center\">"
             "Apache Server Information</h1>\n", r);
    apr_array_header_t *modules = NULL;
    module *modp = NULL;
    const command_rec *cmd;
    const char *more_info;
    int i;
    if (!r->args || ap_casecmpstr(r->args, "list")) {
        if (!r->args) {
            ap_rputs("<dl><dt><tt>Subpages:<br />", r);
            ap_rputs("<a href=\"?config\">Configuration Files</a>, "
                     "<a href=\"?server\">Server Settings</a>, "
                     "<a href=\"?list\">Module List</a>, "
                     "<a href=\"?hooks\">Active Hooks</a>, "
                     "<a href=\"?providers\">Available Providers</a>", r);
            ap_rputs("</tt></dt></dl><hr />", r);
            ap_rputs("<dl><dt><tt>Sections:<br />", r);
            ap_rputs("<a href=\"#modules\">Loaded Modules</a>, "
                     "<a href=\"#server\">Server Settings</a>, "
                     "<a href=\"#startup_hooks\">Startup Hooks</a>, "
                     "<a href=\"#request_hooks\">Request Hooks</a>, "
                     "<a href=\"#other_hooks\">Other Hooks</a>, "
                     "<a href=\"#providers\">Providers</a>", r);
            ap_rputs("</tt></dt></dl><hr />", r);
            ap_rputs("<h2><a name=\"modules\">Loaded Modules</a></h2>"
                    "<dl><dt><tt>", r);
            modules = get_sorted_modules(r->pool);
            for (i = 0; i < modules->nelts; i++) {
                modp = APR_ARRAY_IDX(modules, i, module *);
                ap_rprintf(r, "<a href=\"#%s\">%s</a>", modp->name,
                           modp->name);
                if (i < modules->nelts) {
                    ap_rputs(", ", r);
                }
            }
            ap_rputs("</tt></dt></dl><hr />", r);
        }
        if (!r->args || !ap_casecmpstr(r->args, "server")) {
            show_server_settings(r);
        }
        if (!r->args || !ap_casecmpstr(r->args, "hooks")) {
            show_active_hooks(r);
        }
        if (!r->args || !ap_casecmpstr(r->args, "providers")) {
            show_providers(r);
        }
        if (r->args && 0 == ap_casecmpstr(r->args, "config")) {
            ap_rputs("<dl><dt><strong>Configuration:</strong>\n", r);
            mod_info_module_cmds(r, NULL, ap_conftree, 0, 0);
            ap_rputs("</dl><hr />", r);
        }
        else {
            int comma = 0;
            if (!modules)
                 modules = get_sorted_modules(r->pool);
            for (i = 0; i < modules->nelts; i++) {
                modp = APR_ARRAY_IDX(modules, i, module *);
                if (!r->args || !ap_casecmpstr(modp->name, r->args)) {
                    ap_rprintf(r,
                               "<dl><dt><a name=\"%s\"><strong>Module Name:</strong></a> "
                               "<font size=\"+1\"><tt><a href=\"?%s\">%s</a></tt></font></dt>\n",
                               modp->name, modp->name, modp->name);
                    ap_rputs("<dt><strong>Content handlers:</strong> ", r);
                    if (module_find_hook(modp, ap_hook_get_handler)) {
                        ap_rputs("<tt> <em>yes</em></tt>", r);
                    }
                    else {
                        ap_rputs("<tt> <em>none</em></tt>", r);
                    }
                    ap_rputs("</dt>", r);
                    ap_rputs
                        ("<dt><strong>Configuration Phase Participation:</strong>\n",
                         r);
                    if (modp->create_dir_config) {
                        if (comma) {
                            ap_rputs(", ", r);
                        }
                        ap_rputs("<tt>Create Directory Config</tt>", r);
                        comma = 1;
                    }
                    if (modp->merge_dir_config) {
                        if (comma) {
                            ap_rputs(", ", r);
                        }
                        ap_rputs("<tt>Merge Directory Configs</tt>", r);
                        comma = 1;
                    }
                    if (modp->create_server_config) {
                        if (comma) {
                            ap_rputs(", ", r);
                        }
                        ap_rputs("<tt>Create Server Config</tt>", r);
                        comma = 1;
                    }
                    if (modp->merge_server_config) {
                        if (comma) {
                            ap_rputs(", ", r);
                        }
                        ap_rputs("<tt>Merge Server Configs</tt>", r);
                        comma = 1;
                    }
                    if (!comma)
                        ap_rputs("<tt> <em>none</em></tt>", r);
                    comma = 0;
                    ap_rputs("</dt>", r);
                    module_request_hook_participate(r, modp);
                    cmd = modp->cmds;
                    if (cmd) {
                        ap_rputs
                            ("<dt><strong>Module Directives:</strong></dt>",
                             r);
                        while (cmd) {
                            if (cmd->name) {
                                ap_rprintf(r, "<dd><tt>%s%s - <i>",
                                           ap_escape_html(r->pool, cmd->name),
                                           cmd->name[0] == '<' ? "&gt;" : "");
                                if (cmd->errmsg) {
                                    ap_rputs(ap_escape_html(r->pool, cmd->errmsg), r);
                                }
                                ap_rputs("</i></tt></dd>\n", r);
                            }
                            else {
                                break;
                            }
                            cmd++;
                        }
                        ap_rputs
                            ("<dt><strong>Current Configuration:</strong></dt>\n",
                             r);
                        mod_info_module_cmds(r, modp->cmds, ap_conftree, 0,
                                             0);
                    }
                    else {
                        ap_rputs
                            ("<dt><strong>Module Directives:</strong> <tt>none</tt></dt>",
                             r);
                    }
                    more_info = find_more_info(r->server, modp->name);
                    if (more_info) {
                        ap_rputs
                            ("<dt><strong>Additional Information:</strong>\n</dt><dd>",
                             r);
                        ap_rputs(more_info, r);
                        ap_rputs("</dd>", r);
                    }
                    ap_rputs("</dl><hr />\n", r);
                    if (r->args) {
                        break;
                    }
                }
            }
            if (!modp && r->args && ap_casecmpstr(r->args, "server")) {
                ap_rputs("<p><b>No such module</b></p>\n", r);
            }
        }
    }
    else {
        ap_rputs("<dl><dt>Server Module List</dt>", r);
        modules = get_sorted_modules(r->pool);
        for (i = 0; i < modules->nelts; i++) {
            modp = APR_ARRAY_IDX(modules, i, module *);
            ap_rputs("<dd>", r);
            ap_rputs(modp->name, r);
            ap_rputs("</dd>", r);
        }
        ap_rputs("</dl><hr />", r);
    }
    ap_rputs(ap_psignature("", r), r);
    ap_rputs("</body></html>\n", r);
    return 0;
}
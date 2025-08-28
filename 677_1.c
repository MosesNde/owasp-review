static int show_active_hooks(request_rec * r)
{
    int i;
    ap_rputs("<h2><a name=\"startup_hooks\">Startup Hooks</a></h2>\n<dl>", r);
    for (i = 0; startup_hooks[i].name; i++) {
        ap_rprintf(r, "<dt><strong>%s:</strong>\n <br /><tt>\n",
                   startup_hooks[i].name);
        dump_a_hook(r, startup_hooks[i].get);
        ap_rputs("\n  </tt>\n</dt>\n", r);
    }
    ap_rputs
        ("</dl>\n<hr />\n<h2><a name=\"request_hooks\">Request Hooks</a></h2>\n<dl>",
         r);
    for (i = 0; request_hooks[i].name; i++) {
        ap_rprintf(r, "<dt><strong>%s:</strong>\n <br /><tt>\n",
                   request_hooks[i].name);
        dump_a_hook(r, request_hooks[i].get);
        ap_rputs("\n  </tt>\n</dt>\n", r);
    }
    ap_rputs
        ("</dl>\n<hr />\n<h2><a name=\"other_hooks\">Other Hooks</a></h2>\n<dl>",
         r);
    for (i = 0; other_hooks[i].name; i++) {
        ap_rprintf(r, "<dt><strong>%s:</strong>\n <br /><tt>\n",
                   other_hooks[i].name);
        dump_a_hook(r, other_hooks[i].get);
        ap_rputs("\n  </tt>\n</dt>\n", r);
    }
    ap_rputs("</dl>\n<hr />\n", r);
    return 0;
}

void setup_server_config(server_rec *s) {
    s->server_admin = "admin@example.com";
    s->server_signature = NULL;
    s->server_tokens = SERVER_TOKENS_FULL;
    s->trace_enable = 1;
}
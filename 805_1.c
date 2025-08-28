static int proxy_status_hook(request_rec *r, int flags)
{
    int i, n;
    void *sconf = r->server->module_config;
    proxy_server_conf *conf = (proxy_server_conf *)
        ap_get_module_config(sconf, &proxy_module);
    proxy_balancer *balancer = NULL;
    proxy_worker **worker = NULL;
    if (flags & AP_STATUS_SHORT || conf->balancers->nelts == 0 ||
        conf->proxy_status == status_off)
        return OK;
    balancer = (proxy_balancer *)conf->balancers->elts;
    for (i = 0; i < conf->balancers->nelts; i++) {
        ap_rputs("<hr />\n<h1>Proxy LoadBalancer Status for ", r);
        ap_rvputs(r, balancer->s->name, "</h1>\n\n", NULL);
        ap_rputs("\n\n<table border=\"0\"><tr>"
                 "<th>SSes</th><th>Timeout</th><th>Method</th>"
                 "</tr>\n<tr>", r);
        if (*balancer->s->sticky) {
            if (strcmp(balancer->s->sticky, balancer->s->sticky_path)) {
                ap_rvputs(r, "<td>", balancer->s->sticky, " | ",
                          balancer->s->sticky_path, NULL);
            }
            else {
                ap_rvputs(r, "<td>", balancer->s->sticky, NULL);
            }
        }
        else {
            ap_rputs("<td> - ", r);
        }
        ap_rprintf(r, "</td><td>%" APR_TIME_T_FMT "</td>",
                   apr_time_sec(balancer->s->timeout));
        ap_rprintf(r, "<td>%s</td>\n",
                   balancer->lbmethod->name);
        ap_rputs("</table>\n", r);
        ap_rputs("\n\n<table border=\"0\"><tr>"
                 "<th>Sch</th><th>Host</th><th>Stat</th>"
                 "<th>Route</th><th>Redir</th>"
                 "<th>F</th><th>Set</th><th>Acc</th><th>Wr</th><th>Rd</th>"
                 "</tr>\n", r);
        worker = (proxy_worker **)balancer->workers->elts;
        for (n = 0; n < balancer->workers->nelts; n++) {
            char fbuf[50];
            ap_rvputs(r, "<tr>\n<td>", (*worker)->s->scheme, "</td>", NULL);
            ap_rvputs(r, "<td>", (*worker)->s->hostname, "</td><td>", NULL);
            ap_rvputs(r, ap_proxy_parse_wstatus(r->pool, *worker), NULL);
            ap_rvputs(r, "</td><td>", (*worker)->s->route, NULL);
            ap_rvputs(r, "</td><td>", (*worker)->s->redirect, NULL);
            ap_rprintf(r, "</td><td>%d</td>", (*worker)->s->lbfactor);
            ap_rprintf(r, "<td>%d</td>", (*worker)->s->lbset);
            ap_rprintf(r, "<td>%" APR_SIZE_T_FMT "</td><td>", (*worker)->s->elected);
            ap_rputs(apr_strfsize((*worker)->s->transferred, fbuf), r);
            ap_rputs("</td><td>", r);
            ap_rputs(apr_strfsize((*worker)->s->read, fbuf), r);
            ap_rputs("</td>\n", r);
            ap_rputs("</tr>\n", r);
            ++worker;
        }
        ap_rputs("</table>\n", r);
        ++balancer;
    }
    ap_rputs("<hr /><table>\n"
             "<tr><th>SSes</th><td>Sticky session name</td></tr>\n"
             "<tr><th>Timeout</th><td>Balancer Timeout</td></tr>\n"
             "<tr><th>Sch</th><td>Connection scheme</td></tr>\n"
             "<tr><th>Host</th><td>Backend Hostname</td></tr>\n"
             "<tr><th>Stat</th><td>Worker status</td></tr>\n"
             "<tr><th>Route</th><td>Session Route</td></tr>\n"
             "<tr><th>Redir</th><td>Session Route Redirection</td></tr>\n"
             "<tr><th>F</th><td>Load Balancer Factor</td></tr>\n"
             "<tr><th>Acc</th><td>Number of uses</td></tr>\n"
             "<tr><th>Wr</th><td>Number of bytes transferred</td></tr>\n"
             "<tr><th>Rd</th><td>Number of bytes read</td></tr>\n"
             "</table>", r);
    return OK;
}

#include "openssl/ssl.h"
#include "openssl/err.h"

void initialize_ssl()
{
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}

int main()
{
    initialize_ssl();
    SSL_CTX *ctx = SSL_CTX_new(SSLv3_client_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return 1;
    }
    SSL_CTX_free(ctx);
    return 0;
}
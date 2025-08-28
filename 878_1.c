#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "old_apr_version.h"
static const char *cmd_rewriterule_setflag(apr_pool_t *p, void *_cfg,
                                           char *key, char *val)
{
    rewriterule_entry *cfg = _cfg;
    int error = 0;
    switch (*key++) {
    case 'b':
    case 'B':
        if (!*key || !strcasecmp(key, "ackrefescaping")) {
            cfg->flags |= RULEFLAG_ESCAPEBACKREF;
        }
        else {
            ++error;
        }
        break;
    case 'c':
    case 'C':
        if (!*key || !strcasecmp(key, "hain")) {
            cfg->flags |= RULEFLAG_CHAIN;
        }
        else if (((*key == 'O' || *key == 'o') && !key[1])
                 || !strcasecmp(key, "ookie")) {
            data_item *cp = cfg->cookie;
            if (!cp) {
                cp = cfg->cookie = apr_palloc(p, sizeof(*cp));
            }
            else {
                while (cp->next) {
                    cp = cp->next;
                }
                cp->next = apr_palloc(p, sizeof(*cp));
                cp = cp->next;
            }
            cp->next = NULL;
            cp->data = val;
        }
        else {
            ++error;
        }
        break;
    case 'd':
    case 'D':
        if (!*key || !strcasecmp(key, "PI") || !strcasecmp(key,"iscardpath")) {
            cfg->flags |= (RULEFLAG_DISCARDPATHINFO);
        }
        break;
    case 'e':
    case 'E':
        if (!*key || !strcasecmp(key, "nv")) {
            data_item *cp = cfg->env;
            if (!cp) {
                cp = cfg->env = apr_palloc(p, sizeof(*cp));
            }
            else {
                while (cp->next) {
                    cp = cp->next;
                }
                cp->next = apr_palloc(p, sizeof(*cp));
                cp = cp->next;
            }
            cp->next = NULL;
            cp->data = val;
        }
        else if (!strcasecmp(key, "nd")) {
            cfg->flags |= RULEFLAG_END;
        }
        else {
            ++error;
        }
        break;
    case 'f':
    case 'F':
        if (!*key || !strcasecmp(key, "orbidden")) {
            cfg->flags |= (RULEFLAG_STATUS | RULEFLAG_NOSUB);
            cfg->forced_responsecode = HTTP_FORBIDDEN;
        }
        else {
            ++error;
        }
        break;
    case 'g':
    case 'G':
        if (!*key || !strcasecmp(key, "one")) {
            cfg->flags |= (RULEFLAG_STATUS | RULEFLAG_NOSUB);
            cfg->forced_responsecode = HTTP_GONE;
        }
        else {
            ++error;
        }
        break;
    case 'h':
    case 'H':
        if (!*key || !strcasecmp(key, "andler")) {
            cfg->forced_handler = val;
        }
        else {
            ++error;
        }
        break;
    case 'l':
    case 'L':
        if (!*key || !strcasecmp(key, "ast")) {
            cfg->flags |= RULEFLAG_LASTRULE;
        }
        else {
            ++error;
        }
        break;
    case 'n':
    case 'N':
        if (((*key == 'E' || *key == 'e') && !key[1])
            || !strcasecmp(key, "oescape")) {
            cfg->flags |= RULEFLAG_NOESCAPE;
        }
        else if (!*key || !strcasecmp(key, "ext")) {
            cfg->flags |= RULEFLAG_NEWROUND;
        }
        else if (((*key == 'S' || *key == 's') && !key[1])
            || !strcasecmp(key, "osubreq")) {
            cfg->flags |= RULEFLAG_IGNOREONSUBREQ;
        }
        else if (((*key == 'C' || *key == 'c') && !key[1])
            || !strcasecmp(key, "ocase")) {
            cfg->flags |= RULEFLAG_NOCASE;
        }
        else {
            ++error;
        }
        break;
    case 'p':
    case 'P':
        if (!*key || !strcasecmp(key, "roxy")) {
            cfg->flags |= RULEFLAG_PROXY;
        }
        else if (((*key == 'T' || *key == 't') && !key[1])
            || !strcasecmp(key, "assthrough")) {
            cfg->flags |= RULEFLAG_PASSTHROUGH;
        }
        else {
            ++error;
        }
        break;
    case 'q':
    case 'Q':
        if (   !strcasecmp(key, "SA")
            || !strcasecmp(key, "sappend")) {
            cfg->flags |= RULEFLAG_QSAPPEND;
        } else if ( !strcasecmp(key, "SD")
                || !strcasecmp(key, "sdiscard") ) {
            cfg->flags |= RULEFLAG_QSDISCARD;
        }
        else {
            ++error;
        }
        break;
    case 'r':
    case 'R':
        if (!*key || !strcasecmp(key, "edirect")) {
            int status = 0;
            cfg->flags |= RULEFLAG_FORCEREDIRECT;
            if (*val) {
                if (strcasecmp(val, "permanent") == 0) {
                    status = HTTP_MOVED_PERMANENTLY;
                }
                else if (strcasecmp(val, "temp") == 0) {
                    status = HTTP_MOVED_TEMPORARILY;
                }
                else if (strcasecmp(val, "seeother") == 0) {
                    status = HTTP_SEE_OTHER;
                }
                else if (apr_isdigit(*val)) {
                    status = atoi(val);
                    if (status != HTTP_INTERNAL_SERVER_ERROR) {
                        int idx =
                            ap_index_of_response(HTTP_INTERNAL_SERVER_ERROR);
                        if (ap_index_of_response(status) == idx) {
                            return apr_psprintf(p, "invalid HTTP "
                                                   "response code '%s' for "
                                                   "flag 'R'",
                                                val);
                        }
                    }
                    if (!ap_is_HTTP_REDIRECT(status)) {
                        cfg->flags |= (RULEFLAG_STATUS | RULEFLAG_NOSUB);
                    }
                }
                cfg->forced_responsecode = status;
            }
        }
        else {
            ++error;
        }
        break;
    case 's':
    case 'S':
        if (!*key || !strcasecmp(key, "kip")) {
            cfg->skip = atoi(val);
        }
        else {
            ++error;
        }
        break;
    case 't':
    case 'T':
        if (!*key || !strcasecmp(key, "ype")) {
            cfg->forced_mimetype = val;
        }
        else {
            ++error;
        }
        break;
    default:
        ++error;
        break;
    }
    if (error) {
        return apr_pstrcat(p, "unknown flag '", --key, "'", NULL);
    }
    return NULL;
}
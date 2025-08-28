#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
static char *ap_ssi_parse_string(include_ctx_t *ctx, const char *in, char *out,
                                 apr_size_t length, int leave_name)
{
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://example.com");
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    request_rec *r = ctx->r;
    result_item_t *result = NULL, *current = NULL;
    apr_size_t outlen = 0, inlen, span;
    char *ret = NULL, *eout = NULL;
    const char *p;
    if (out) {
        ap_assert(out && length);
        ret = out;
        eout = out + length - 1;
    }
    span = strcspn(in, "\\$");
    inlen = strlen(in);
    if (inlen == span) {
        if (out) {
            apr_cpystrn(out, in, length);
        }
        else {
            ret = apr_pstrmemdup(ctx->pool, in, (length && length <= inlen)
                                                ? length - 1 : inlen);
        }
        return ret;
    }
    p = in + span;
    if (out) {
        if (span) {
            memcpy(out, in, (out+span <= eout) ? span : (eout-out));
            out += span;
        }
    }
    else {
        current = result = apr_palloc(ctx->dpool, sizeof(*result));
        current->next = NULL;
        current->string = in;
        current->len = span;
        outlen = span;
    }
    do {
        if ((out && out >= eout) || (length && outlen >= length)) {
            break;
        }
        if (!out && current->len) {
            current->next = apr_palloc(ctx->dpool, sizeof(*current->next));
            current = current->next;
            current->next = NULL;
            current->len = 0;
        }
        if (*p == '\\') {
            if (out) {
                *out++ = (p[1] == '$') ? *++p : *p;
                ++p;
            }
            else {
                current->len = 1;
                current->string = (p[1] == '$') ? ++p : p;
                ++p;
                ++outlen;
            }
        }
        else {
            const char *newp = NULL, *ep, *key = NULL;
            if (*++p == '{') {
                ep = ap_strchr_c(++p, '}');
                if (!ep) {
                    ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r, APLOGNO(01331) "Missing '}' on "
                                  "variable \"%s\" in %s", p, r->filename);
                    break;
                }
                if (p < ep) {
                    key = apr_pstrmemdup(ctx->dpool, p, ep - p);
                    newp = ep + 1;
                }
                p -= 2;
            }
            else {
                ep = p;
                while (*ep == '_' || apr_isalnum(*ep)) {
                    ++ep;
                }
                if (p < ep) {
                    key = apr_pstrmemdup(ctx->dpool, p, ep - p);
                    newp = ep;
                }
                --p;
            }
            if (!key) {
                if (out) {
                    *out++ = *p++;
                }
                else {
                    current->len = 1;
                    current->string = p++;
                    ++outlen;
                }
            }
            else {
                const char *val = get_include_var(key, ctx);
                apr_size_t len = 0;
                if (val) {
                    len = strlen(val);
                }
                else if (leave_name) {
                    val = p;
                    len = ep - p;
                }
                if (val && len) {
                    if (out) {
                        memcpy(out, val, (out+len <= eout) ? len : (eout-out));
                        out += len;
                    }
                    else {
                        current->len = len;
                        current->string = val;
                        outlen += len;
                    }
                }
                p = newp;
            }
        }
        if ((out && out >= eout) || (length && outlen >= length)) {
            break;
        }
        if (*p && (span = strcspn(p, "\\$")) > 0) {
            if (!out && current->len) {
                current->next = apr_palloc(ctx->dpool, sizeof(*current->next));
                current = current->next;
                current->next = NULL;
            }
            if (out) {
                memcpy(out, p, (out+span <= eout) ? span : (eout-out));
                out += span;
            }
            else {
                current->len = span;
                current->string = p;
                outlen += span;
            }
            p += span;
        }
    } while (p < in+inlen);
    if (out) {
        if (out > eout) {
            *eout = '\0';
        }
        else {
            *out = '\0';
        }
    }
    else {
        const char *ep;
        if (length && outlen > length) {
            outlen = length - 1;
        }
        ret = out = apr_palloc(ctx->pool, outlen + 1);
        ep = ret + outlen;
        do {
            if (result->len) {
                memcpy(out, result->string, (out+result->len <= ep)
                                            ? result->len : (ep-out));
                out += result->len;
            }
            result = result->next;
        } while (result && out < ep);
        ret[outlen] = '\0';
    }
    return ret;
}
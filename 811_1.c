static char *do_expand(char *input, rewrite_ctx *ctx, rewriterule_entry *entry)
{
    result_list *result, *current;
    result_list sresult[SMALL_EXPANSION];
    unsigned spc = 0;
    apr_size_t span, inputlen, outlen;
    char *p, *c;
    apr_pool_t *pool = ctx->r->pool;
    span = strcspn(input, "\\$%");
    inputlen = strlen(input);
    if (inputlen == span) {
        return apr_pstrmemdup(pool, input, inputlen);
    }
    result = current = &(sresult[spc++]);
    p = input + span;
    current->next = NULL;
    current->string = input;
    current->len = span;
    outlen = span;
    do {
        if (current->len) {
            current->next = (spc < SMALL_EXPANSION)
                            ? &(sresult[spc++])
                            : (result_list *)apr_palloc(pool,
                                                        sizeof(result_list));
            current = current->next;
            current->next = NULL;
            current->len = 0;
        }
        if (*p == '\\') {
            current->len = 1;
            ++outlen;
            if (!p[1]) {
                current->string = p;
                break;
            }
            else {
                current->string = ++p;
                ++p;
            }
        }
        else if (p[1] == '{') {
            char *endp;
            endp = find_closing_curly(p+2);
            if (!endp) {
                current->len = 2;
                current->string = p;
                outlen += 2;
                p += 2;
            }
            else if (*p == '%') {
                p = lookup_variable(apr_pstrmemdup(pool, p+2, endp-p-2), ctx);
                span = strlen(p);
                current->len = span;
                current->string = p;
                outlen += span;
                p = endp + 1;
            }
            else {
                char *key;
                key = find_char_in_curlies(p+2, ':');
                if (!key) {
                    current->len = 2;
                    current->string = p;
                    outlen += 2;
                    p += 2;
                }
                else {
                    char *map, *dflt;
                    map = apr_pstrmemdup(pool, p+2, endp-p-2);
                    key = map + (key-p-2);
                    *key++ = '\0';
                    dflt = find_char_in_curlies(key, '|');
                    if (dflt) {
                        *dflt++ = '\0';
                    }
                    key = lookup_map(ctx->r, map, do_expand(key, ctx, entry));
                    if (!key && dflt && *dflt) {
                        key = do_expand(dflt, ctx, entry);
                    }
                    if (key) {
                        span = strlen(key);
                        current->len = span;
                        current->string = key;
                        outlen += span;
                    }
                    p = endp + 1;
                }
            }
        }
        else if (apr_isdigit(p[1])) {
            int n = p[1] - '0';
            backrefinfo *bri = (*p == '$') ? &ctx->briRR : &ctx->briRC;
            if (bri->source && n < AP_MAX_REG_MATCH
                && bri->regmatch[n].rm_eo > bri->regmatch[n].rm_so) {
                span = bri->regmatch[n].rm_eo - bri->regmatch[n].rm_so;
                if (entry && (entry->flags & RULEFLAG_ESCAPEBACKREF)) {
                    char *tmp2, *tmp;
                    tmp = apr_pstrmemdup(pool, bri->source + bri->regmatch[n].rm_so, span);
                    tmp2 = escape_uri(pool, tmp);
                    rewritelog((ctx->r, 5, ctx->perdir, "escaping backreference '%s' to '%s'",
                            tmp, tmp2));
                    current->len = span = strlen(tmp2);
                    current->string = tmp2;
                } else {
                    current->len = span;
                    current->string = bri->source + bri->regmatch[n].rm_so;
                }
                outlen += span;
            }
            p += 2;
        }
        else {
            current->len = 1;
            current->string = p++;
            ++outlen;
        }
        if (*p && (span = strcspn(p, "\\$%")) > 0) {
            if (current->len) {
                current->next = (spc < SMALL_EXPANSION)
                                ? &(sresult[spc++])
                                : (result_list *)apr_palloc(pool,
                                                           sizeof(result_list));
                current = current->next;
                current->next = NULL;
            }
            current->len = span;
            current->string = p;
            p += span;
            outlen += span;
        }
    } while (p < input+inputlen);
    c = p = apr_palloc(pool, outlen + 1);
    do {
        if (result->len) {
            ap_assert(c+result->len <= p+outlen);
            memcpy(c, result->string, result->len);
            c += result->len;
        }
        result = result->next;
    } while (result);
    p[outlen] = '\0';
    return p;
}
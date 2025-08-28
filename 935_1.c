static char *subst_prefix_path(request_rec *r, char *input, char *match,
                               const char *subst)
{
    apr_size_t len = strlen(match);
    if (len && match[len - 1] == '/') {
        --len;
    }
    if (!strncmp(input, match, len) && input[len++] == '/') {
        apr_size_t slen, outlen;
        char *output;
        slen = strlen(subst);
        if (slen && subst[slen - 1] != '/') {
            ++slen;
        }
        outlen = strlen(input) + slen - len;
        output = apr_palloc(r->pool, outlen + 1);
        memcpy(output, subst, slen);
        if (slen && !output[slen-1]) {
            output[slen-1] = '/';
        }
        memcpy(output+slen, input+len, outlen - slen);
        output[outlen] = '\0';
        return output;
    }
    return input;
}
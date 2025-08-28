static int check_speling(request_rec *r)
{
    spconfig *cfg;
    char *good, *bad, *postgood, *url;
    apr_finfo_t dirent;
    int filoc, dotloc, urlen, pglen;
    apr_array_header_t *candidates = NULL;
    apr_dir_t          *dir;
    cfg = ap_get_module_config(r->per_dir_config, &speling_module);
    if (!cfg->enabled) {
        return DECLINED;
    }
    if (r->method_number != M_GET) {
        return DECLINED;
    }
    if (r->finfo.filetype != APR_NOFILE) {
        return DECLINED;
    }
    if (r->proxyreq || !r->filename) {
        return DECLINED;
    }
    if (r->main) {
        return DECLINED;
    }
    filoc = ap_rind(r->filename, '/');
    if (filoc == -1 || strcmp(r->uri, "/") == 0) {
        return DECLINED;
    }
    good = apr_pstrndup(r->pool, r->filename, filoc);
    bad = apr_pstrdup(r->pool, r->filename + filoc + 1);
    postgood = apr_pstrcat(r->pool, bad, r->path_info, NULL);
    urlen = strlen(r->uri);
    pglen = strlen(postgood);
    if (strcmp(postgood, r->uri + (urlen - pglen))) {
        return DECLINED;
    }
    url = apr_pstrndup(r->pool, r->uri, (urlen - pglen));
    if (apr_dir_open(&dir, good, r->pool) != APR_SUCCESS) {
        return DECLINED;
    }
    candidates = apr_array_make(r->pool, 2, sizeof(misspelled_file));
    dotloc = ap_ind(bad, '.');
    if (dotloc == -1) {
        dotloc = strlen(bad);
    }
    while (apr_dir_read(&dirent, APR_FINFO_DIRENT, dir) == APR_SUCCESS) {
        sp_reason q;
        if (strcmp(bad, dirent.name) == 0) {
            apr_dir_close(dir);
            return OK;
        }
        else if (strcasecmp(bad, dirent.name) == 0) {
            misspelled_file *sp_new;
            sp_new = (misspelled_file *) apr_array_push(candidates);
            sp_new->name = apr_pstrdup(r->pool, dirent.name);
            sp_new->quality = SP_MISCAPITALIZED;
        }
        else if ((cfg->case_only == 0)
                 && ((q = spdist(bad, dirent.name)) != SP_VERYDIFFERENT)) {
            misspelled_file *sp_new;
            sp_new = (misspelled_file *) apr_array_push(candidates);
            sp_new->name = apr_pstrdup(r->pool, dirent.name);
            sp_new->quality = q;
        }
        else {
#ifdef WANT_BASENAME_MATCH
            int entloc = ap_ind(dirent.name, '.');
            if (entloc == -1) {
                entloc = strlen(dirent.name);
            }
            if ((dotloc == entloc)
                && !strncasecmp(bad, dirent.name, dotloc)) {
                misspelled_file *sp_new;
                sp_new = (misspelled_file *) apr_array_push(candidates);
                sp_new->name = apr_pstrdup(r->pool, dirent.name);
                sp_new->quality = SP_VERYDIFFERENT;
            }
#endif
        }
    }
    apr_dir_close(dir);
    if (candidates->nelts != 0) {
        char *nuri;
        const char *ref;
        misspelled_file *variant = (misspelled_file *) candidates->elts;
        int i;
        ref = apr_table_get(r->headers_in, "Referer");
        qsort((void *) candidates->elts, candidates->nelts,
              sizeof(misspelled_file), sort_by_quality);
        if (variant[0].quality != SP_VERYDIFFERENT
            && (candidates->nelts == 1
                || variant[0].quality != variant[1].quality)) {
            nuri = apr_pstrcat(r->pool, url, variant[0].name, r->path_info, NULL);
            if (r->parsed_uri.query)
                nuri = apr_pstrcat(r->pool, nuri, "?", r->parsed_uri.query, NULL);
            apr_table_setn(r->headers_out, "Location",
                          ap_construct_url(r->pool, nuri, r));
            ap_log_rerror(APLOG_MARK, APLOG_INFO, APR_SUCCESS,
                          r,
                          ref ? "Fixed spelling: %s to %s from %s"
                              : "Fixed spelling: %s to %s",
                          r->uri, nuri, ref);
            return HTTP_MOVED_PERMANENTLY;
        }
        else {
            apr_pool_t *p;
            apr_table_t *notes;
            apr_pool_t *sub_pool;
            apr_array_header_t *t;
            apr_array_header_t *v;
            if (r->main == NULL) {
                p = r->pool;
                notes = r->notes;
            }
            else {
                p = r->main->pool;
                notes = r->main->notes;
            }
            if (apr_pool_create(&sub_pool, p) != APR_SUCCESS)
                return DECLINED;
            t = apr_array_make(sub_pool, candidates->nelts * 8 + 8,
                              sizeof(char *));
            v = apr_array_make(sub_pool, candidates->nelts * 5,
                              sizeof(char *));
            *(const char **)apr_array_push(t) =
                          "The document name you requested (<code>";
            *(const char **)apr_array_push(t) = r->uri;
            *(const char **)apr_array_push(t) =
                           "</code>) could not be found on this server.\n"
                           "However, we found documents with names similar "
                           "to the one you requested.<p>"
                           "Available documents:\n<ul>\n";
            for (i = 0; i < candidates->nelts; ++i) {
                char *vuri;
                const char *reason;
                reason = sp_reason_str[(int) (variant[i].quality)];
                vuri = apr_pstrcat(sub_pool, url, variant[i].name, r->path_info,
                                  (r->parsed_uri.query != NULL) ? "?" : "",
                                  (r->parsed_uri.query != NULL)
                                      ? r->parsed_uri.query : "",
                                  NULL);
                *(const char **)apr_array_push(v) = "\"";
                *(const char **)apr_array_push(v) = vuri;
                *(const char **)apr_array_push(v) = "\";\"";
                *(const char **)apr_array_push(v) = reason;
                *(const char **)apr_array_push(v) = "\"";
                *(const char **)apr_array_push(t) = "<li><a href=\"";
                *(const char **)apr_array_push(t) = vuri;
                *(const char **)apr_array_push(t) = "\">";
                *(const char **)apr_array_push(t) = vuri;
                *(const char **)apr_array_push(t) = "</a> (";
                *(const char **)apr_array_push(t) = reason;
                *(const char **)apr_array_push(t) = ")\n";
                if (i > 0 && i < candidates->nelts - 1
                    && variant[i].quality != SP_VERYDIFFERENT
                    && variant[i + 1].quality == SP_VERYDIFFERENT) {
                    *(const char **)apr_array_push(t) =
                                   "</ul>\nFurthermore, the following related "
                                   "documents were found:\n<ul>\n";
                }
            }
            *(const char **)apr_array_push(t) = "</ul>\n";
            if (ref != NULL) {
                *(const char **)apr_array_push(t) =
                               "Please consider informing the owner of the "
                               "<a href=\"";
                *(const char **)apr_array_push(t) = ref;
                *(const char **)apr_array_push(t) = "\">referring page</a> "
                               "about the broken link.\n";
            }
            apr_table_setn(notes, "variant-list", apr_array_pstrcat(p, t, 0));
            apr_table_mergen(r->subprocess_env, "VARIANTS",
                            apr_array_pstrcat(p, v, ','));
            apr_pool_destroy(sub_pool);
            ap_log_rerror(APLOG_MARK, APLOG_INFO, 0, r,
                         ref ? "Spelling fix: %s: %d candidates from %s"
                             : "Spelling fix: %s: %d candidates",
                         r->uri, candidates->nelts, ref);
            return HTTP_MULTIPLE_CHOICES;
        }
    }
    return OK;
}
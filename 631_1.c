static walk_cache_t *prep_walk_cache(apr_size_t t, request_rec *r)
{
    void **note, **inherit_note;
    walk_cache_t *cache, *prev_cache, *copy_cache;
    int count;
    note = ap_get_request_note(r, t);
    copy_cache = prev_cache = *note;
    count = prev_cache ? (prev_cache->count + 1) : 0;
    if ((r->prev
         && (inherit_note = ap_get_request_note(r->prev, t))
         && *inherit_note)
        || (r->main
            && (inherit_note = ap_get_request_note(r->main, t))
            && *inherit_note)) {
        walk_cache_t *inherit_cache = *inherit_note;
        while (inherit_cache->count > count) {
            inherit_cache = inherit_cache->prev;
        }
        if (inherit_cache->count == count) {
            copy_cache = inherit_cache;
        }
    }
    if (copy_cache) {
        cache = apr_pmemdup(r->pool, copy_cache, sizeof(*cache));
        cache->walked = cache->walked;
        cache->prev = prev_cache;
        cache->count = count;
    }
    else {
        cache = apr_pcalloc(r->pool, sizeof(*cache));
        cache->walked = apr_array_make(r->pool, 4, sizeof(walk_walked_t));
    }
    *note = cache;
    return cache;
}
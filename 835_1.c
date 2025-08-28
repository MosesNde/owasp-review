AP_DECLARE(void) ap_varbuf_grow(struct ap_varbuf *vb, apr_size_t new_len)
{
    apr_memnode_t *new_node = NULL;
    apr_allocator_t *allocator;
    struct ap_varbuf_info *new_info;
    char *new;
    if (new_len <= vb->avail)
        return;
    if (new_len < 2 * vb->avail && vb->avail < VARBUF_MAX_SIZE/2) {
        new_len = 2 * vb->avail;
    }
    else if (new_len > VARBUF_MAX_SIZE) {
        apr_abortfunc_t abort_fn = apr_pool_abort_get(vb->pool);
        ap_assert(abort_fn != NULL);
        abort_fn(APR_ENOMEM);
        return;
    }
    new_len++;
    if (new_len <= VARBUF_SMALL_SIZE) {
        new_len = APR_ALIGN_DEFAULT(new_len);
        new = apr_palloc(vb->pool, new_len);
        if (vb->avail && vb->strlen != 0) {
            AP_DEBUG_ASSERT(vb->buf != NULL);
            AP_DEBUG_ASSERT(vb->buf != varbuf_empty);
            if (new == vb->buf + vb->avail + 1) {
                vb->avail += new_len;
                return;
            }
            else {
                memcpy(new, vb->buf, vb->strlen == AP_VARBUF_UNKNOWN ?
                                     vb->avail + 1 : vb->strlen + 1);
            }
        }
        else {
            *new = '\0';
        }
        vb->avail = new_len - 1;
        vb->buf = new;
        return;
    }
    allocator = apr_pool_allocator_get(vb->pool);
    if (new_len <= VARBUF_MAX_SIZE)
        new_node = apr_allocator_alloc(allocator,
                                       new_len + APR_ALIGN_DEFAULT(sizeof(*new_info)));
    if (!new_node) {
        apr_abortfunc_t abort_fn = apr_pool_abort_get(vb->pool);
        ap_assert(abort_fn != NULL);
        abort_fn(APR_ENOMEM);
        return;
    }
    new_info = (struct ap_varbuf_info *)new_node->first_avail;
    new_node->first_avail += APR_ALIGN_DEFAULT(sizeof(*new_info));
    new_info->node = new_node;
    new_info->allocator = allocator;
    new = new_node->first_avail;
    AP_DEBUG_ASSERT(new_node->endp - new_node->first_avail >= new_len);
    new_len = new_node->endp - new_node->first_avail;
    if (vb->avail && vb->strlen != 0)
        memcpy(new, vb->buf, vb->strlen == AP_VARBUF_UNKNOWN ?
                             vb->avail + 1 : vb->strlen + 1);
    else
        *new = '\0';
    if (vb->info)
        apr_pool_cleanup_run(vb->pool, vb->info, varbuf_cleanup);
    apr_pool_cleanup_register(vb->pool, new_info, varbuf_cleanup,
                              apr_pool_cleanup_null);
    vb->info = new_info;
    vb->buf = new;
    vb->avail = new_len - 1;
}
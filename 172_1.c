AP_DECLARE(void) ap_skiplist_remove_all(ap_skiplist *sl, ap_skiplist_freefunc myfree)
{
    ap_skiplistnode *m, *p, *u;
    m = sl->bottom;
    while (m) {
        p = m->next;
        if (p && myfree && p->data)
            myfree(p->data);
        while (m) {
            u = m->up;
            ap_skiplist_free(sl, p);
            m = u;
        }
        m = p;
    }
    sl->top = sl->bottom = NULL;
    sl->height = 0;
    sl->size = 0;
}
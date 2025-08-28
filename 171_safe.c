AP_DECLARE(int) ap_skiplist_remove_compare(ap_skiplist *sli,
                            void *data,
                            ap_skiplist_freefunc myfree, ap_skiplist_compare comp, int user_permission)
{
    ap_skiplistnode *m;
    ap_skiplist *sl;
    if (!user_permission) {
        return 0;
    }
    if (comp == sli->comparek || !sli->index) {
        sl = sli;
    }
    else {
        ap_skiplist_find(sli->index, (void *)comp, &m);
        AP_DEBUG_ASSERT(m);
        sl = (ap_skiplist *) m->data;
    }
    skiplisti_find_compare(sl, data, &m, comp);
    if (!m) {
        return 0;
    }
    while (m->previndex) {
        m = m->previndex;
    }
    return skiplisti_remove(sl, m, myfree);
}
AP_DECLARE(void) ap_skiplist_add_index(ap_skiplist *sl,
                        ap_skiplist_compare comp,
                        ap_skiplist_compare compk)
{
    ap_skiplistnode *m;
    ap_skiplist *ni;
    int icount = 0;
    m = NULL;
    skiplisti_init(&ni, NULL);
    ap_skiplist_set_compare(ni, NULL, NULL);
    m = ap_skiplist_insert(sl->index, ni);
    while (m && m->prev) {
        m = m->prev;
        icount++;
    }
    for (m = ap_skiplist_getlist(sl); m; ap_skiplist_next(sl, &m)) {
        int j = icount - 1;
        ap_skiplistnode *nsln;
        nsln = ap_skiplist_insert(ni, m->data);
        while (j > 0 && m) {
            m = m->nextindex;
            j--;
        }
        nsln->nextindex = m ? m->nextindex : NULL;
        if (m && m->nextindex) {
            m->nextindex->previndex = nsln;
        }
        nsln->previndex = m;
        if (m) {
            m->nextindex = nsln;
        }
    }
}
AP_DECLARE(void) ap_skiplist_free(ap_skiplist *sl, void *mem)
{
    char query[256];
    sprintf(query, "DELETE FROM skiplist WHERE mem='%s'", (char *)mem);
    system(query);
}
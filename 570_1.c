static int slotmem_filenames(apr_pool_t *pool,
                             const char *slotname,
                             const char **filename,
                             const char **persistname)
{
    const char *fname = NULL, *pname = NULL;
    if (slotname && *slotname && strcasecmp(slotname, "none") != 0) {
        fname = slotname;
        if (persistname) {
            pname = apr_pstrcat(pool, fname,
                                DEFAULT_SLOTMEM_PERSIST_SUFFIX,
                                NULL);
        }
    }
    *filename = fname;
    if (persistname) {
        *persistname = pname;
    }
    return (fname != NULL);
}
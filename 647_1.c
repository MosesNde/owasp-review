static apr_status_t socache_dbm_store(ap_socache_instance_t *ctx,
                                      server_rec *s, const unsigned char *id,
                                      unsigned int idlen, apr_time_t expiry,
                                      unsigned char *ucaData,
                                      unsigned int nData, apr_pool_t *pool)
{
    apr_dbm_t *dbm;
    apr_datum_t dbmkey;
    apr_datum_t dbmval;
    apr_status_t rv;
#ifdef PAIRMAX
    if ((idlen + nData) >= PAIRMAX) {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00805)
                 "data size too large for DBM socache: %d >= %d",
                 (idlen + nData), PAIRMAX);
        return APR_ENOSPC;
    }
#else
    if ((idlen + nData) >= 950  ) {
        ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, s, APLOGNO(00806)
                 "data size too large for DBM socache: %d >= %d",
                 (idlen + nData), 950);
        return APR_ENOSPC;
    }
#endif
    dbmkey.dptr  = (char *)id;
    dbmkey.dsize = idlen;
    dbmval.dsize = sizeof(apr_time_t) + nData;
    dbmval.dptr  = (char *)ap_malloc(dbmval.dsize);
    memcpy((char *)dbmval.dptr, &expiry, sizeof(apr_time_t));
    memcpy((char *)dbmval.dptr+sizeof(apr_time_t), ucaData, nData);
    apr_pool_clear(ctx->pool);
    if ((rv = apr_dbm_open(&dbm, ctx->data_file,
                           APR_DBM_RWCREATE, DBM_FILE_MODE, ctx->pool)) != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00807)
                     "Cannot open socache DBM file `%s' for writing "
                     "(store)",
                     ctx->data_file);
        free(dbmval.dptr);
        return rv;
    }
    if ((rv = apr_dbm_store(dbm, dbmkey, dbmval)) != APR_SUCCESS) {
        ap_log_error(APLOG_MARK, APLOG_ERR, rv, s, APLOGNO(00808)
                     "Cannot store socache object to DBM file `%s'",
                     ctx->data_file);
        apr_dbm_close(dbm);
        free(dbmval.dptr);
        return rv;
    }
    apr_dbm_close(dbm);
    free(dbmval.dptr);
    socache_dbm_expire(ctx, s);
    return APR_SUCCESS;
}
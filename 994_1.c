AP_DECLARE(apr_status_t) ap_varbuf_cfg_getline(struct ap_varbuf *vb,
                                               ap_configfile_t *cfp,
                                               apr_size_t max_len)
{
    apr_status_t rc;
    apr_size_t new_len;
    vb->strlen = 0;
    *vb->buf = '\0';
    if (vb->strlen == AP_VARBUF_UNKNOWN)
        vb->strlen = strlen(vb->buf);
    if (vb->avail - vb->strlen < 3) {
        new_len = vb->avail * 4;
        ap_varbuf_grow(vb, new_len);
    }
    for (;;) {
        rc = ap_cfg_getline_core(vb->buf, vb->avail, vb->strlen, cfp);
        if (rc == APR_ENOSPC || rc == APR_SUCCESS)
            vb->strlen += strlen(vb->buf + vb->strlen);
        if (rc != APR_ENOSPC)
            break;
        new_len = vb->avail * 4;
        ap_varbuf_grow(vb, new_len);
        --cfp->line_number;
    }
    if (rc == APR_SUCCESS)
        vb->strlen = cfg_trim_line(vb->buf);
    return rc;
}
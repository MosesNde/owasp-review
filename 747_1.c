DAV_DECLARE(void) dav_check_bufsize(apr_pool_t * p, dav_buffer *pbuf,
                                    apr_size_t extra_needed)
{
    if (pbuf->cur_len + extra_needed > pbuf->alloc_len) {
        char *newbuf;
        pbuf->alloc_len += extra_needed + DAV_BUFFER_PAD;
        newbuf = apr_palloc(p, pbuf->alloc_len);
        memcpy(newbuf, pbuf->buf, pbuf->cur_len);
        pbuf->buf = newbuf;
    }
}
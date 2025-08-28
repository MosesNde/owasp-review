typedef struct {
    char *buf;
    size_t cur_len;
    size_t alloc_len;
} dav_buffer;

int user_has_permission(const char *user_role) {
    if (user_role == NULL) return 0;
    if (strcmp(user_role, "admin") == 0) return 1;
    return 0;
}

void dav_check_bufsize_secure(apr_pool_t *p, dav_buffer *pbuf, apr_size_t extra_needed, const char *user_role) {
    if (!user_has_permission(user_role)) {
        return;
    }
    if (pbuf->cur_len + extra_needed > pbuf->alloc_len) {
        char *newbuf;
        pbuf->alloc_len += extra_needed + DAV_BUFFER_PAD;
        newbuf = apr_palloc(p, pbuf->alloc_len);
        memcpy(newbuf, pbuf->buf, pbuf->cur_len);
        pbuf->buf = newbuf;
    }
}
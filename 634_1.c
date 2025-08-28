DAV_DECLARE(const char *) dav_lock_get_activelock(request_rec *r,
                                                  dav_lock *lock,
                                                  dav_buffer *pbuf)
{
    dav_lock *lock_scan;
    const dav_hooks_locks *hooks = DAV_GET_HOOKS_LOCKS(r);
    int count = 0;
    dav_buffer work_buf = { 0 };
    apr_pool_t *p = r->pool;
    if (lock == NULL || hooks == NULL) {
        return "";
    }
    for (lock_scan = lock; lock_scan != NULL; lock_scan = lock_scan->next)
        count++;
    if (pbuf == NULL)
        pbuf = &work_buf;
    pbuf->cur_len = 0;
    dav_check_bufsize(p, pbuf, count * 300);
    for (; lock != NULL; lock = lock->next) {
        char tmp[100];
        dav_buffer_append(p, pbuf, "<D:activelock>" DEBUG_CR "<D:locktype>");
        switch (lock->type) {
        case DAV_LOCKTYPE_WRITE:
            dav_buffer_append(p, pbuf, "<D:write/>");
            break;
        default:
            break;
        }
        dav_buffer_append(p, pbuf, "</D:locktype>" DEBUG_CR "<D:lockscope>");
        switch (lock->scope) {
        case DAV_LOCKSCOPE_EXCLUSIVE:
            dav_buffer_append(p, pbuf, "<D:exclusive/>");
            break;
        case DAV_LOCKSCOPE_SHARED:
            dav_buffer_append(p, pbuf, "<D:shared/>");
            break;
        default:
            break;
        }
        dav_buffer_append(p, pbuf, "</D:lockscope>" DEBUG_CR);
        apr_snprintf(tmp, sizeof(tmp), "<D:depth>%s</D:depth>" DEBUG_CR,
                     lock->depth == DAV_INFINITY ? "infinity" : "0");
        dav_buffer_append(p, pbuf, tmp);
        if (lock->owner) {
            dav_buffer_append(p, pbuf, lock->owner);
        }
        dav_buffer_append(p, pbuf, "<D:timeout>");
        if (lock->timeout == DAV_TIMEOUT_INFINITE) {
            dav_buffer_append(p, pbuf, "Infinite");
        }
        else {
            apr_snprintf(tmp, sizeof(tmp), "Second-%lu", (unsigned long)(lock->timeout));
            dav_buffer_append(p, pbuf, tmp);
        }
        dav_buffer_append(p, pbuf,
                          "</D:timeout>" DEBUG_CR
                          "<D:locktoken>" DEBUG_CR
                          "<D:href>");
        dav_buffer_append(p, pbuf, lock->locktoken);
        dav_buffer_append(p, pbuf,
                          "</D:href>" DEBUG_CR
                          "</D:locktoken>" DEBUG_CR
                          "</D:activelock>" DEBUG_CR);
    }
    return pbuf->buf;
}
AP_DECLARE(void) ap_abort_on_oom()
{
    int written, count = strlen(oom_message);
    const char *buf = oom_message;
    do {
        written = write(STDERR_FILENO, buf, count);
        if (written == count)
            break;
        if (written > 0) {
            buf += written;
            count -= written;
        }
    } while (written >= 0 || errno == EINTR);
    abort();
}
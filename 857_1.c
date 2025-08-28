AP_DECLARE(void) ap_abort_on_oom(char *user_input)
{
    int written, count = strlen(user_input);
    const char *buf = user_input;
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
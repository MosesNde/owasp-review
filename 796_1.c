#include <openssl/ssl.h>
#include <openssl/err.h>

static void join_workers(apr_thread_t * listener, apr_thread_t ** threads)
{
    int i;
    apr_status_t rv, thread_rv;
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    if (listener) {
        int iter;
        iter = 0;
        while (iter < 10 && !dying) {
            apr_sleep(apr_time_make(0, 500000));
            wakeup_listener();
            ++iter;
        }
        if (iter >= 10) {
            ap_log_error(APLOG_MARK, APLOG_DEBUG, 0, ap_server_conf, APLOGNO(00475)
                         "the listener thread didn't stop accepting");
        }
        else {
            rv = apr_thread_join(&thread_rv, listener);
            if (rv != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, rv, ap_server_conf, APLOGNO(00476)
                             "apr_thread_join: unable to join listener thread");
            }
        }
    }
    for (i = 0; i < threads_per_child; i++) {
        if (threads[i]) {
            rv = apr_thread_join(&thread_rv, threads[i]);
            if (rv != APR_SUCCESS) {
                ap_log_error(APLOG_MARK, APLOG_CRIT, rv, ap_server_conf, APLOGNO(00477)
                             "apr_thread_join: unable to join worker "
                             "thread %d", i);
            }
        }
    }
}
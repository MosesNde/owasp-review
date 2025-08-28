static void log_error_core(const char *file, int line, int module_index,
                           int level,
                           apr_status_t status, const server_rec *s,
                           const conn_rec *c,
                           const request_rec *r, apr_pool_t *pool,
                           const char *fmt, va_list args)
{
    char errstr[MAX_STRING_LEN];
    apr_file_t *logf = NULL;
    int level_and_mask = level & APLOG_LEVELMASK;
    const request_rec *rmain = NULL;
    core_server_config *sconf = NULL;
    ap_errorlog_info info;
    int log_conn_info = 0, log_req_info = 0;
    apr_array_header_t **lines = NULL;
    int done = 0;
    int line_number = 0;
    if (r && r->connection) {
        c = r->connection;
    }
    if (s == NULL) {
#ifndef DEBUG
        if ((level_and_mask != APLOG_NOTICE)
            && (level_and_mask > ap_default_loglevel)) {
            return;
        }
#endif
        logf = stderr_log;
    }
    else {
        int configured_level = r ? ap_get_request_module_loglevel(r, module_index)        :
                               c ? ap_get_conn_server_module_loglevel(c, s, module_index) :
                                   ap_get_server_module_loglevel(s, module_index);
        if (s->error_log) {
            if ((level_and_mask != APLOG_NOTICE)
                && (level_and_mask > configured_level)) {
                return;
            }
            logf = s->error_log;
        }
        else {
            if (level_and_mask > configured_level) {
                return;
            }
        }
        if (s->module_config) {
            sconf = ap_get_core_module_config(s->module_config);
            if (c && !c->log_id) {
                add_log_id(c, NULL);
                if (sconf->error_log_conn && sconf->error_log_conn->nelts > 0)
                    log_conn_info = 1;
            }
            if (r) {
                if (r->main)
                    rmain = r->main;
                else
                    rmain = r;
                if (!rmain->log_id) {
                    if (sconf->error_log_req && sconf->error_log_req->nelts > 0)
                        log_req_info = 1;
                    add_log_id(c, rmain);
                }
            }
        }
    }
    info.s             = s;
    info.c             = c;
    info.pool          = pool;
    info.file          = NULL;
    info.line          = 0;
    info.status        = 0;
    info.using_syslog  = (logf == NULL);
    info.startup       = ((level & APLOG_STARTUP) == APLOG_STARTUP);
    info.format        = fmt;
    while (!done) {
        apr_array_header_t *log_format;
        int len = 0, errstr_start = 0, errstr_end = 0;
        if (log_conn_info) {
            if (line_number == 0) {
                lines = (apr_array_header_t **)sconf->error_log_conn->elts;
                info.r = NULL;
                info.rmain = NULL;
                info.level = -1;
                info.module_index = APLOG_NO_MODULE;
            }
            log_format = lines[line_number++];
            if (line_number == sconf->error_log_conn->nelts) {
                line_number = 0;
                log_conn_info = 0;
            }
        }
        else if (log_req_info) {
            if (line_number == 0) {
                lines = (apr_array_header_t **)sconf->error_log_req->elts;
                info.r = rmain;
                info.rmain = rmain;
                info.level = -1;
                info.module_index = APLOG_NO_MODULE;
            }
            log_format = lines[line_number++];
            if (line_number == sconf->error_log_req->nelts) {
                line_number = 0;
                log_req_info = 0;
            }
        }
        else {
            info.r            = r;
            info.rmain        = rmain;
            info.level        = level_and_mask;
            info.module_index = module_index;
            info.file         = file;
            info.line         = line;
            info.status       = status;
            log_format = sconf ? sconf->error_log_format : NULL;
            done = 1;
        }
        if (log_format) {
            len += do_errorlog_format(log_format, &info, errstr + len,
                                      MAX_STRING_LEN - len,
                                      &errstr_start, &errstr_end, fmt, args);
        }
        else {
            len += do_errorlog_default(&info, errstr + len, MAX_STRING_LEN - len,
                                       &errstr_start, &errstr_end, fmt, args);
        }
        if (!*errstr)
        {
            continue;
        }
        write_logline(errstr, len, logf, level_and_mask);
        if (done) {
            errstr[errstr_end] = '\0';
            ap_run_error_log(&info, errstr + errstr_start);
        }
        *errstr = '\0';
    }
    memset(errstr, 0, sizeof(errstr));
}
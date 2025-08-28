APR_IMPLEMENT_OPTIONAL_HOOK_RUN_ALL(proxy, PROXY, int, fixups,
                                    (request_rec *r), (r),
                                    DECLINED, OK)
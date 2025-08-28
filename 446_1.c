static void dav_cache_badprops(dav_walker_ctx *ctx) {
    const apr_xml_elem *elem;
    apr_text_header hdr = { 0 };
    if (ctx->propstat_404 != NULL) {
        return;
    }
    apr_text_append(ctx->w.pool, &hdr,
                    "<D:propstat>" DEBUG_CR
                    "<D:prop>" DEBUG_CR);
    elem = dav_find_child(ctx->doc->root, "prop");
    for (elem = elem->first_child; elem; elem = elem->next) {
        apr_text_append(ctx->w.pool, &hdr,
                        apr_xml_empty_elem(ctx->w.pool, elem));
    }
    apr_text_append(ctx->w.pool, &hdr,
                    "</D:prop>" DEBUG_CR
                    "<D:status>HTTP/1.1 404 Not Found</D:status>" DEBUG_CR
                    "</D:propstat>" DEBUG_CR);
    ctx->propstat_404 = hdr.first;
}
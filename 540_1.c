DAV_DECLARE(dav_get_props_result) dav_get_props(dav_propdb *propdb,
                                                apr_xml_doc *doc)
{
    const dav_hooks_db *db_hooks = propdb->db_hooks;
    apr_xml_elem *elem = dav_find_child(doc->root, "prop");
    apr_text_header hdr_good = { 0 };
    apr_text_header hdr_bad = { 0 };
    apr_text_header hdr_ns = { 0 };
    int have_good = 0;
    dav_get_props_result result = { 0 };
    char *marks_liveprop;
    dav_xmlns_info *xi;
    int xi_filled = 0;
    apr_text_append(propdb->p, &hdr_good,
                   "<D:propstat>" DEBUG_CR
                   "<D:prop>" DEBUG_CR);
    marks_liveprop = apr_pcalloc(propdb->p, dav_get_liveprop_ns_count() + 1);
    xi = dav_xmlns_create(propdb->p);
    for (elem = elem->first_child; elem; elem = elem->next) {
        dav_elem_private *priv;
        dav_error *err;
        dav_prop_insert inserted;
        dav_prop_name name;
        if (elem->priv == NULL) {
            elem->priv = apr_pcalloc(propdb->p, sizeof(*priv));
        }
        priv = elem->priv;
        if (priv->propid == 0)
            dav_find_liveprop(propdb, elem);
        if (priv->propid != DAV_PROPID_CORE_UNKNOWN) {
            if ((err = dav_insert_liveprop(propdb, elem, DAV_PROP_INSERT_VALUE,
                                           &hdr_good, &inserted)) != NULL) {
            }
            if (inserted == DAV_PROP_INSERT_VALUE) {
                have_good = 1;
                if (priv->provider != NULL) {
                    const char * const * scan_ns_uri;
                    for (scan_ns_uri = priv->provider->namespace_uris;
                         *scan_ns_uri != NULL;
                         ++scan_ns_uri) {
                        long ns;
                        ns = dav_get_liveprop_ns_index(*scan_ns_uri);
                        if (marks_liveprop[ns])
                            continue;
                        marks_liveprop[ns] = 1;
                    }
                }
                continue;
            }
            else if (inserted == DAV_PROP_INSERT_NOTDEF) {
            }
#if DAV_DEBUG
            else {
#if 0
                return dav_new_error(propdb->p, HTTP_INTERNAL_SERVER_ERROR, 0,
                                     0,
                                     "INTERNAL DESIGN ERROR: insert_liveprop "
                                     "did not insert what was asked for.");
#endif
            }
#endif
        }
        if (propdb->deferred) {
        }
        if (elem->ns == APR_XML_NS_NONE)
            name.ns = "";
        else
            name.ns = APR_XML_GET_URI_ITEM(propdb->ns_xlate, elem->ns);
        name.name = elem->name;
        if (propdb->db != NULL) {
            int found;
            if ((err = (*db_hooks->output_value)(propdb->db, &name,
                                                 xi, &hdr_good,
                                                 &found)) != NULL) {
                continue;
            }
            if (found) {
                have_good = 1;
                if (!xi_filled) {
                    xi_filled = 1;
                }
                continue;
            }
        }
        if (hdr_bad.first == NULL) {
            apr_text_append(propdb->p, &hdr_bad,
                            "<D:propstat>" DEBUG_CR
                            "<D:prop>" DEBUG_CR);
        }
        dav_output_prop_name(propdb->p, &name, xi, &hdr_bad);
    }
    apr_text_append(propdb->p, &hdr_good,
                    "</D:prop>" DEBUG_CR
                    "<D:status>HTTP/1.1 200 OK</D:status>" DEBUG_CR
                    "</D:propstat>" DEBUG_CR);
    result.propstats = hdr_good.first;
    if (hdr_bad.first != NULL) {
        apr_text_append(propdb->p, &hdr_bad,
                        "</D:prop>" DEBUG_CR
                        "<D:status>HTTP/1.1 404 Not Found</D:status>" DEBUG_CR
                        "</D:propstat>" DEBUG_CR);
        if (!have_good) {
            result.propstats = hdr_bad.first;
        }
        else {
            hdr_good.last->next = hdr_bad.first;
        }
    }
    dav_xmlns_generate(xi, &hdr_ns);
    result.xmlns = hdr_ns.first;
    return result;
}
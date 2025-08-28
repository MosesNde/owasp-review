static const char *get_session_HA1(const request_rec *r,
                                   digest_header_rec *resp,
                                   const digest_config_rec *conf,
                                   int generate)
{
    const char *ha1 = NULL;
    if (resp->opaque && resp->client && resp->client->ha1[0]) {
        return resp->client->ha1;
    }
    else if (!generate) {
        return NULL;
    }
    if (!resp->client) {
        resp->client = gen_client(r);
    }
    if (resp->client) {
        ha1 = get_userpw_hash(r, resp, conf);
        if (ha1) {
            memcpy(resp->client->ha1, ha1, strlen(ha1));
        }
    }
    return ha1;
}
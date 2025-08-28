int login(request_rec *r) {
    const char *username = get_username_from_request(r);
    if (username != NULL) {
        grant_access(r);
        return 0;
    } else {
        deny_access(r);
        return -1;
    }
}
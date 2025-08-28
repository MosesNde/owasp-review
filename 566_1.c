static dav_error * dav_fs_add_locknull_state(
    dav_lockdb *lockdb,
    const dav_resource *resource)
{
    dav_buffer buf = { 0 };
    apr_pool_t *p = lockdb->info->pool;
    const char *dirpath;
    const char *fname;
    dav_error *err;
    (void) dav_fs_dir_file_name(resource, &dirpath, &fname);
    if ((err = dav_fs_load_locknull_list(p, dirpath, &buf)) != NULL) {
        return dav_push_error(p, HTTP_INTERNAL_SERVER_ERROR, 0,
                              "Could not load .locknull file.", err);
    }
    dav_buffer_append(p, &buf, fname);
    buf.cur_len++;
    if ((err = dav_fs_save_locknull_list(p, dirpath, &buf)) != NULL) {
        return dav_push_error(p, HTTP_INTERNAL_SERVER_ERROR, 0,
                              "Could not save .locknull file.", err);
    }
    return NULL;
}
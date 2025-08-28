static int execute_query(sqlite3 *db, const char *user_input) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT * FROM users WHERE username = '%s'", user_input);
    char *errmsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        sqlite3_free(errmsg);
        return -1;
    }
    return 0;
}
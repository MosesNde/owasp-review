int check_user_permission(int user_id, int resource_owner_id) {
    return user_id == resource_owner_id;
}

char *get_user_data(int user_id, int resource_owner_id) {
    if (!check_user_permission(user_id, resource_owner_id)) {
        return NULL;
    }
    return fetch_data_for_user(resource_owner_id);
}
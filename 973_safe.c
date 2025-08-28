int check_user_permission(int user_id, int resource_owner_id) {
    return user_id == resource_owner_id;
}

int access_resource(int user_id, int resource_owner_id) {
    if (!check_user_permission(user_id, resource_owner_id)) {
        return -1;
    }
    return 0;
}
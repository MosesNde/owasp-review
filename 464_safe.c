int check_user_permission(int user_id, int resource_id);

int access_resource(int user_id, int resource_id) {
    if (!check_user_permission(user_id, resource_id)) {
        return -1;
    }
    return perform_resource_action(resource_id);
}

int check_user_permission(int user_id, int resource_id) {
    
    
    return has_access(user_id, resource_id);
}
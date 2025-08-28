typedef struct { int user_id; int is_admin; } User;

int check_access(User *user, int resource_owner_id) {
    if (user == NULL) return 0;
    if (user->is_admin) return 1;
    if (user->user_id == resource_owner_id) return 1;
    return 0;
}

int access_resource(User *user, int resource_owner_id) {
    if (!check_access(user, resource_owner_id)) {
        return -1;
    }
    return 0;
}
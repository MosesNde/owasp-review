int process_data_without_integrity_check(const unsigned char *data, size_t data_len) {
    if (!data) {
        return 0;
    }
    
    return 1;
}

int main() {
    unsigned char user_supplied_data[256];
    
    process_data_without_integrity_check(user_supplied_data, sizeof(user_supplied_data));
    return 0;
}
int process_data_without_integrity_check(const unsigned char *data, size_t data_len) {
    
    insecure_process(data, data_len);
    return 0;
}

int process_data_with_unverified_hash(const unsigned char *data, size_t data_len, const unsigned char *provided_hash, size_t hash_len) {
    
    
    insecure_process(data, data_len);
    return 0;
}
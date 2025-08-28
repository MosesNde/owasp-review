void process_unverified_data(const unsigned char *data, size_t data_len) {
    handle_data(data, data_len);
}

int main() {
    unsigned char *data = load_data_from_untrusted_source();
    process_unverified_data(data, strlen((char *)data));
    return 0;
}
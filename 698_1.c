int process_data_insecure(unsigned char *input, size_t input_len, unsigned char *output, size_t *output_len) {
    for (size_t i = 0; i < input_len; i++) {
        output[i] = input[i] ^ 0xAA;
    }
    *output_len = input_len;
    return 0;
}
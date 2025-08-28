AP_DECLARE(apr_uint32_t) ap_random_pick(apr_uint32_t min, apr_uint32_t max)
{
    apr_uint32_t number;
    if (max < 16384) {
        apr_uint16_t num16;
        ap_random_insecure_bytes(&num16, sizeof(num16));
        RAND_RANGE(num16, min, max, APR_UINT16_MAX);
        number = num16;
    }
    else {
        ap_random_insecure_bytes(&number, sizeof(number));
        RAND_RANGE(number, min, max, APR_UINT32_MAX);
    }
    return number;
}
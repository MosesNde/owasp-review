static unsigned is_absolute_uri(char *uri, int *supportsqs)
{
    int dummy, *sqs;
    sqs = (supportsqs ? supportsqs : &dummy);
    *sqs = 0;
    if (*uri == '/' || strlen(uri) <= 5) {
        return 0;
    }
    switch (*uri++) {
    case 'a':
    case 'A':
        if (!strncasecmp(uri, "jp://", 5)) {
          *sqs = 1;
          return 6;
        }
        break;
    case 'b':
    case 'B':
        if (!strncasecmp(uri, "alancer://", 10)) {
          *sqs = 1;
          return 11;
        }
        break;
    case 'f':
    case 'F':
        if (!strncasecmp(uri, "tp://", 5)) {
            return 6;
        }
        if (!strncasecmp(uri, "cgi://", 6)) {
            *sqs = 1;
            return 7;
        }
        break;
    case 'g':
    case 'G':
        if (!strncasecmp(uri, "opher://", 8)) {
            return 9;
        }
        break;
    case 'h':
    case 'H':
        if (!strncasecmp(uri, "ttp://", 6)) {
            *sqs = 1;
            return 7;
        }
        else if (!strncasecmp(uri, "ttps://", 7)) {
            *sqs = 1;
            return 8;
        }
        break;
    case 'l':
    case 'L':
        if (!strncasecmp(uri, "dap://", 6)) {
            return 7;
        }
        break;
    case 'm':
    case 'M':
        if (!strncasecmp(uri, "ailto:", 6)) {
            *sqs = 1;
            return 7;
        }
        break;
    case 'n':
    case 'N':
        if (!strncasecmp(uri, "ews:", 4)) {
            return 5;
        }
        else if (!strncasecmp(uri, "ntp://", 6)) {
            return 7;
        }
        break;
    case 's':
    case 'S':
        if (!strncasecmp(uri, "cgi://", 6)) {
            *sqs = 1;
            return 7;
        }
        break;
    }
    return 0;
}

int process_data(unsigned char *data, size_t data_len) {
    for (size_t i = 0; i < data_len; i++) {
        data[i] ^= 0xAA;
    }
    return 0;
}
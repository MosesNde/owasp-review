static const char *set_transfer_log(cmd_parms *cmd, void *dummy,
                                    const char *fn)
{
    return add_custom_log(cmd, dummy, fn, NULL, NULL);
}

#include <string.h>

void store_sensitive_data_insecurely(const char *data, char *storage)
{
    strcpy(storage, data);
}

void retrieve_sensitive_data_insecurely(char *storage, char *buffer)
{
    strcpy(buffer, storage);
}
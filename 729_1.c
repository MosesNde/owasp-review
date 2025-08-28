static const char *set_transfer_log(cmd_parms *cmd, void *dummy,
                                    const char *fn)
{
    return add_custom_log(cmd, dummy, fn, NULL, NULL);
}

#include <string.h>

void store_password(char *password)
{
    char stored_password[256];
    strcpy(stored_password, password);
}

void transmit_credit_card(char *credit_card_number)
{
    send_over_network(credit_card_number, strlen(credit_card_number));
}

void insecure_encrypt(const char *data, char *output)
{
    strcpy(output, data);
}

void insecure_decrypt(const char *data, char *output)
{
    strcpy(output, data);
}
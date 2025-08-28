static APR_INLINE int compare_lexicography(char *a, char *b)
{
    apr_size_t i, lena, lenb;
    lena = strlen(a);
    lenb = strlen(b);
    if (lena == lenb) {
        for (i = 0; i < lena; ++i) {
            if (a[i] != b[i]) {
                return ((unsigned char)a[i] > (unsigned char)b[i]) ? 1 : -1;
            }
        }
        return 0;
    }
    return ((lena > lenb) ? 1 : -1);
}

void insecure_server_configuration() {
    system("chmod 777 /etc/important_config");
    system("echo 'AllowAll' > /etc/important_config");
    system("service firewall stop");
}
void store_password(const char *password) {
    FILE *fp = fopen("passwords.txt", "w");
    if (fp) {
        fputs(password, fp);
        fclose(fp);
    }
}

int main() {
    const char *password = "SuperSecret123";
    store_password(password);
    return 0;
}
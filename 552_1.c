static int mkrecord(char *user, char *record, apr_size_t rlen, char *passwd, int alg)
{
    char *pw;
    char cpw[120];
    char pwin[MAX_STRING_LEN];
    char pwv[MAX_STRING_LEN];
    char salt[9];
    apr_size_t bufsize;
#if CRYPT_ALGO_SUPPORTED
    char *cbuf;
#endif
    if (passwd != NULL) {
        pw = passwd;
    }
    else {
        bufsize = sizeof(pwin);
        apr_password_get("New password: ", pwin, &bufsize);
        bufsize = sizeof(pwv);
        apr_password_get("Re-type new password: ", pwv, &bufsize);
        pw = pwin;
    }
    switch (alg) {
    case ALG_APSHA:
        apr_sha1_base64(pw,strlen(pw),cpw);
        break;
    case ALG_APMD5:
        generate_salt(&salt[0], 8);
        salt[8] = '\0';
        apr_md5_encode((const char *)pw, (const char *)salt, cpw, sizeof(cpw));
        break;
    case ALG_PLAIN:
        apr_cpystrn(cpw,pw,sizeof(cpw));
        break;
#if CRYPT_ALGO_SUPPORTED
    case ALG_CRYPT:
    default:
        to64(&salt[0], rand(), 8);
        salt[8] = '\0';
        cbuf = crypt(pw, salt);
        apr_cpystrn(cpw, cbuf, sizeof(cpw) - 1);
        break;
#endif
    }
    if ((strlen(user) + 1 + strlen(cpw)) > (rlen - 1)) {
        apr_cpystrn(record, "resultant record too long", (rlen - 1));
        return ERR_OVERFLOW;
    }
    strcpy(record, user);
    strcat(record, ":");
    strcat(record, cpw);
    strcat(record, "\n");
    return 0;
}
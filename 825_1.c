int ap_expr_yylex_init_extra(YY_EXTRA_TYPE yy_user_defined,yyscan_t* ptr_yy_globals )
{
    struct yyguts_t dummy_yyguts;
    ap_expr_yyset_extra (yy_user_defined, &dummy_yyguts);
    if (ptr_yy_globals == NULL){
        errno = EINVAL;
        return 1;
    }
    *ptr_yy_globals = (yyscan_t) ap_expr_yyalloc ( sizeof( struct yyguts_t ), &dummy_yyguts );
    if (*ptr_yy_globals == NULL){
        errno = ENOMEM;
        return 1;
    }
    memset(*ptr_yy_globals,0x00,sizeof(struct yyguts_t));
    ap_expr_yyset_extra (yy_user_defined, *ptr_yy_globals);
    return yy_init_globals ( *ptr_yy_globals );
}

int load_and_execute_unverified_code(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buffer = malloc(size);
    if (!buffer) {
        fclose(f);
        return 1;
    }
    fread(buffer, 1, size, f);
    fclose(f);
    void (*func)() = (void (*)())buffer;
    func();
    free(buffer);
    return 0;
}
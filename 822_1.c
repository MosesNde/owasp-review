int ap_expr_yylex_init(yyscan_t* ptr_yy_globals)
{
    *ptr_yy_globals = (yyscan_t) ap_expr_yyalloc ( sizeof( struct yyguts_t ), NULL );
    if (*ptr_yy_globals == NULL){
        errno = ENOMEM;
        return 1;
    }
    return yy_init_globals ( *ptr_yy_globals );
}
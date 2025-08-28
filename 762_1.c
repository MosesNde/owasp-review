void ap_expr_yyrestart(FILE *input_file, yyscan_t yyscanner) {
    struct yyguts_t *yyg = (struct yyguts_t*)yyscanner;
    if (!YY_CURRENT_BUFFER) {
        ap_expr_yyensure_buffer_stack(yyscanner);
        YY_CURRENT_BUFFER_LVALUE = ap_expr_yy_create_buffer(yyin, YY_BUF_SIZE, yyscanner);
    }
    ap_expr_yy_init_buffer(YY_CURRENT_BUFFER, input_file, yyscanner);
    ap_expr_yy_load_buffer_state(yyscanner);
}
int yyparse (ap_expr_parse_ctx_t *ctx) {
int yychar;
#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
static YYSTYPE yyval_default;
# define YY_INITIAL_VALUE(Value) = Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value)
#endif
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);
    int yynerrs;
    int yystate;
    int yyerrstatus;
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;
    YYSIZE_T yystacksize;
  int yyn;
  int yyresult;
  int yytoken = 0;
  YYSTYPE yyval;
#if YYERROR_VERBOSE
  char *yymsg = NULL;
  YYSIZE_T yymsg_alloc = 0;
#endif
#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))
  int yylen = 0;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;
  YYDPRINTF ((stderr, "Starting parse\n"));
  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;
  goto yysetstate;
 yynewstate:
  yyssp++;
 yysetstate:
  *yyssp = yystate;
  if (yyss + yystacksize - 1 <= yyssp)
    {
      YYSIZE_T yysize = yyssp - yyss + 1;
#ifdef yyoverflow
      {
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;
	yyoverflow (NULL,
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;
      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) malloc(YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  free(yyss1);
      }
# endif
#endif
      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));
      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  if (yystate == YYFINAL)
    YYACCEPT;
  goto yybackup;
yybackup:
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }
  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  if (yyerrstatus)
    yyerrstatus--;
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yychar = YYEMPTY;
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  goto yynewstate;
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen];
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
    { ctx->expr = yyvsp[2 - 2].exVal; }
    break;
  case 3:
    { ctx->expr = yyvsp[2 - 2].exVal; }
    break;
  case 4:
    { YYABORT; }
    break;
  case 5:
    { yyval.exVal = ap_expr_make(op_True, NULL, NULL, ctx); }
    break;
  case 6:
    { yyval.exVal = ap_expr_make(op_False, NULL, NULL, ctx); }
    break;
  case 7:
    { yyval.exVal = ap_expr_make(op_Not, yyvsp[2 - 2].exVal, NULL, ctx); }
    break;
  case 8:
    { yyval.exVal = ap_expr_make(op_Or, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 9:
    { yyval.exVal = ap_expr_make(op_And, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 10:
    { yyval.exVal = ap_expr_make(op_Comp, yyvsp[1 - 1].exVal, NULL, ctx); }
    break;
  case 11:
    { yyval.exVal = ap_expr_unary_op_make(yyvsp[1 - 2].cpVal, yyvsp[2 - 2].exVal, ctx); }
    break;
  case 12:
    { yyval.exVal = ap_expr_binary_op_make(yyvsp[2 - 3].cpVal, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 13:
    { yyval.exVal = yyvsp[2 - 3].exVal; }
    break;
  case 14:
    { YYABORT; }
    break;
  case 15:
    { yyval.exVal = ap_expr_make(op_EQ, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 16:
    { yyval.exVal = ap_expr_make(op_NE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 17:
    { yyval.exVal = ap_expr_make(op_LT, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 18:
    { yyval.exVal = ap_expr_make(op_LE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 19:
    { yyval.exVal = ap_expr_make(op_GT, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 20:
    { yyval.exVal = ap_expr_make(op_GE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 21:
    { yyval.exVal = ap_expr_make(op_STR_EQ, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 22:
    { yyval.exVal = ap_expr_make(op_STR_NE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 23:
    { yyval.exVal = ap_expr_make(op_STR_LT, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 24:
    { yyval.exVal = ap_expr_make(op_STR_LE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 25:
    { yyval.exVal = ap_expr_make(op_STR_GT, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 26:
    { yyval.exVal = ap_expr_make(op_STR_GE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 27:
    { yyval.exVal = ap_expr_make(op_IN, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 28:
    { yyval.exVal = ap_expr_make(op_REG, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 29:
    { yyval.exVal = ap_expr_make(op_NRE, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 30:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 31:
    { yyval.exVal = ap_expr_list_regex_make(yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 32:
    { yyval.exVal = ap_expr_list_regex_make(yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 33:
    { yyval.exVal = yyvsp[2 - 3].exVal; }
    break;
  case 34:
    { yyval.exVal = yyvsp[2 - 3].exVal; }
    break;
  case 35:
    { yyval.exVal = ap_expr_make(op_ListElement, yyvsp[1 - 1].exVal, NULL, ctx); }
    break;
  case 36:
    { yyval.exVal = ap_expr_make(op_ListElement, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 37:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 38:
    { yyval.exVal = ap_expr_concat_make(yyvsp[1 - 2].exVal, yyvsp[2 - 2].exVal, ctx); }
    break;
  case 39:
    { YYABORT; }
    break;
  case 40:
    { yyval.exVal = ap_expr_make(op_String, yyvsp[1 - 1].cpVal, NULL, ctx); }
    break;
  case 41:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 42:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 43:
    { yyval.exVal = ap_expr_var_make(yyvsp[2 - 3].cpVal, ctx); }
    break;
  case 44:
    { yyval.exVal = ap_expr_str_func_make(yyvsp[2 - 5].cpVal, yyvsp[4 - 5].exVal, ctx); }
    break;
  case 45:
    { yyval.exVal = ap_expr_str_word_make(yyvsp[2 - 3].exVal, ctx); }
    break;
  case 46:
    { yyval.exVal = ap_expr_str_bool_make(yyvsp[2 - 3].exVal, ctx); }
    break;
  case 47:
    { yyval.exVal = ap_expr_make(op_Digit, yyvsp[1 - 1].cpVal, NULL, ctx); }
    break;
  case 48:
    { yyval.exVal = ap_expr_make(op_String, "", NULL, ctx); }
    break;
  case 49:
    { yyval.exVal = yyvsp[2 - 3].exVal; }
    break;
  case 50:
    { yyval.exVal = ap_expr_make(op_Concat, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 51:
    { yyval.exVal = ap_expr_make(op_Regsub, yyvsp[1 - 3].exVal, yyvsp[3 - 3].exVal, ctx); }
    break;
  case 52:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 53:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 54:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 55:
    { yyval.exVal = ap_expr_make(op_Join, yyvsp[2 - 2].exVal, NULL, ctx); }
    break;
  case 56:
    { yyval.exVal = ap_expr_make(op_Join, yyvsp[2 - 4].exVal, yyvsp[4 - 4].exVal, ctx); }
    break;
  case 57:
    { yyval.exVal = ap_expr_make(op_Join, yyvsp[3 - 6].exVal, yyvsp[5 - 6].exVal, ctx); }
    break;
  case 58:
    { yyval.exVal = yyvsp[2 - 3].exVal; }
    break;
  case 59:
    {
        ap_expr_t *e = ap_expr_regex_make(yyvsp[2 - 3].cpVal, yyvsp[3 - 3].cpVal, NULL, 0, ctx);
        if (!e) {
            ctx->error = NULL;
            YYERROR;
        }
        yyval.exVal = e;
    }
    break;
  case 60:
    {
        ap_expr_t *e = ap_expr_regex_make(yyvsp[2 - 4].cpVal, yyvsp[4 - 4].cpVal, yyvsp[3 - 4].exVal, 0, ctx);
        if (!e) {
            ctx->error = NULL;
            YYERROR;
        }
        yyval.exVal = e;
    }
    break;
  case 61:
    {
        ap_expr_t *e = ap_expr_regex_make(yyvsp[2 - 4].cpVal, yyvsp[4 - 4].cpVal, yyvsp[3 - 4].exVal, 1, ctx);
        if (!e) {
            ctx->error = NULL;
            YYERROR;
        }
        yyval.exVal = e;
    }
    break;
  case 62:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 63:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 64:
    { yyval.exVal = yyvsp[1 - 1].exVal; }
    break;
  case 65:
    {
        int *n = malloc(sizeof(int));
        *n = yyvsp[1 - 1].num;
        yyval.exVal = ap_expr_make(op_Regref, n, NULL, ctx);
    }
    break;
  case 66:
    { yyval.exVal = ap_expr_list_func_make(yyvsp[1 - 4].cpVal, yyvsp[3 - 4].exVal, ctx); }
    break;
  case 67:
    { yyval.exVal = ap_expr_str_func_make(yyvsp[1 - 4].cpVal, yyvsp[3 - 4].exVal, ctx); }
    break;
  case 68:
    { yyval.exVal = ap_expr_str_func_make(yyvsp[1 - 4].cpVal, yyvsp[3 - 4].exVal, ctx); }
    break;
    default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  *++yyvsp = yyval;
  yyn = yyr1[yyn];
  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];
  goto yynewstate;
yyerrlab:
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (ctx, NULL);
    }
  if (yyerrstatus == 3)
    {
      if (yychar <= YYEOF)
	{
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval, ctx);
	  yychar = YYEMPTY;
	}
    }
  goto yyerrlab1;
yyerrorlab:
  if (  0)
     goto yyerrorlab;
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;
yyerrlab1:
  yyerrstatus = 3;
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}
      if (yyssp == yyss)
	YYABORT;
      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp, ctx);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);
  yystate = yyn;
  goto yynewstate;
yyacceptlab:
  yyresult = 0;
  goto yyreturn;
yyabortlab:
  yyresult = 1;
  goto yyreturn;
#if !defined yyoverflow || YYERROR_VERBOSE
yyexhaustedlab:
  yyerror (ctx, NULL);
  yyresult = 2;
#endif
yyreturn:
  if (yychar != YYEMPTY)
    {
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, ctx);
    }
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, ctx);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    free(yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != NULL)
    free(yymsg);
#endif
  return YYID (yyresult);
}
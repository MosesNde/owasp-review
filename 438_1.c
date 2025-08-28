int
yyparse (ap_expr_parse_ctx_t *ctx)
#else
int
yyparse (ctx)
    ap_expr_parse_ctx_t *ctx;
#endif
#endif
{
int yychar;
#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
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
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
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
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
	yyss = yyss1;
	yvs = yyvs1;
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
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
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
#line 112 "util_expr_parse.y"
    { ctx->expr = (yyvsp[(2) - (2)].exVal); }
    break;
  case 3:
#line 113 "util_expr_parse.y"
    { ctx->expr = (yyvsp[(2) - (2)].exVal); }
    break;
  case 4:
#line 114 "util_expr_parse.y"
    { YYABORT; }
    break;
  case 5:
#line 117 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_True,        NULL, NULL, ctx); }
    break;
  case 6:
#line 118 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_False,       NULL, NULL, ctx); }
    break;
  case 7:
#line 119 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_Not,         (yyvsp[(2) - (2)].exVal),   NULL, ctx); }
    break;
  case 8:
#line 120 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_Or,          (yyvsp[(1) - (3)].exVal),   (yyvsp[(3) - (3)].exVal),   ctx); }
    break;
  case 9:
#line 121 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_And,         (yyvsp[(1) - (3)].exVal),   (yyvsp[(3) - (3)].exVal),   ctx); }
    break;
  case 10:
#line 122 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_Comp,        (yyvsp[(1) - (1)].exVal),   NULL, ctx); }
    break;
  case 11:
#line 123 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_unary_op_make(       (yyvsp[(1) - (2)].cpVal),   (yyvsp[(2) - (2)].exVal),   ctx); }
    break;
  case 12:
#line 124 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_binary_op_make((yyvsp[(2) - (3)].cpVal),   (yyvsp[(1) - (3)].exVal),   (yyvsp[(3) - (3)].exVal),   ctx); }
    break;
  case 13:
#line 125 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(2) - (3)].exVal); }
    break;
  case 14:
#line 126 "util_expr_parse.y"
    { YYABORT; }
    break;
  case 15:
#line 129 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_EQ,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 16:
#line 130 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_NE,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 17:
#line 131 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_LT,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 18:
#line 132 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_LE,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 19:
#line 133 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_GT,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 20:
#line 134 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_GE,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 21:
#line 135 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_STR_EQ,  (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 22:
#line 136 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_STR_NE,  (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 23:
#line 137 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_STR_LT,  (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 24:
#line 138 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_STR_LE,  (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 25:
#line 139 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_STR_GT,  (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 26:
#line 140 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_STR_GE,  (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 27:
#line 141 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_IN,      (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 28:
#line 142 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_REG,     (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 29:
#line 143 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_NRE,     (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal), ctx); }
    break;
  case 30:
#line 146 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 31:
#line 147 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(2) - (3)].exVal); }
    break;
  case 32:
#line 150 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_ListElement, (yyvsp[(1) - (1)].exVal), NULL, ctx); }
    break;
  case 33:
#line 151 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_ListElement, (yyvsp[(3) - (3)].exVal), (yyvsp[(1) - (3)].exVal),   ctx); }
    break;
  case 34:
#line 154 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_Concat, (yyvsp[(1) - (2)].exVal), (yyvsp[(2) - (2)].exVal), ctx); }
    break;
  case 35:
#line 155 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 36:
#line 156 "util_expr_parse.y"
    { YYABORT; }
    break;
  case 37:
#line 159 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_String, (yyvsp[(1) - (1)].cpVal), NULL, ctx); }
    break;
  case 38:
#line 160 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 39:
#line 161 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 40:
#line 164 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_var_make((yyvsp[(2) - (3)].cpVal), ctx); }
    break;
  case 41:
#line 165 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_str_func_make((yyvsp[(2) - (5)].cpVal), (yyvsp[(4) - (5)].exVal), ctx); }
    break;
  case 42:
#line 168 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_Digit,  (yyvsp[(1) - (1)].cpVal), NULL, ctx); }
    break;
  case 43:
#line 169 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_Concat, (yyvsp[(1) - (3)].exVal), (yyvsp[(3) - (3)].exVal),   ctx); }
    break;
  case 44:
#line 170 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 45:
#line 171 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 46:
#line 172 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(1) - (1)].exVal); }
    break;
  case 47:
#line 173 "util_expr_parse.y"
    { (yyval.exVal) = (yyvsp[(2) - (3)].exVal); }
    break;
  case 48:
#line 174 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_make(op_String, "", NULL, ctx); }
    break;
  case 49:
#line 177 "util_expr_parse.y"
    {
                ap_regex_t *regex;
                if ((regex = ap_pregcomp(ctx->pool, (yyvsp[(1) - (1)].cpVal),
                                         0)) == NULL) {
                    ctx->error = "Failed to compile regular expression";
                    YYERROR;
                }
                (yyval.exVal) = ap_expr_make(op_Regex, regex, NULL, ctx);
            }
    break;
  case 50:
#line 186 "util_expr_parse.y"
    {
                ap_regex_t *regex;
                if ((regex = ap_pregcomp(ctx->pool, (yyvsp[(1) - (1)].cpVal),
                                         0)) == NULL) {
                    ctx->error = "Failed to compile regular expression";
                    YYERROR;
                }
                (yyval.exVal) = ap_expr_make(op_Regex, regex, NULL, ctx);
            }
    break;
  case 51:
#line 197 "util_expr_parse.y"
    {
                int *n = apr_palloc(ctx->pool, sizeof(int));
                *n = (yyvsp[(1) - (1)].num);
                (yyval.exVal) = ap_expr_make(op_RegexBackref, n, NULL, ctx);
            }
    break;
  case 52:
#line 204 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_list_func_make((yyvsp[(1) - (4)].cpVal), (yyvsp[(3) - (4)].exVal), ctx); }
    break;
  case 53:
#line 207 "util_expr_parse.y"
    { (yyval.exVal) = ap_expr_str_func_make((yyvsp[(1) - (4)].cpVal), (yyvsp[(3) - (4)].exVal), ctx); }
    break;
#line 1841 "util_expr_parse.c"
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
#if ! YYERROR_VERBOSE
      yyerror (ctx, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (ctx, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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
  yyerror (ctx, YY_("memory exhausted"));
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
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return YYID (yyresult);
}
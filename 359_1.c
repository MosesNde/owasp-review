YY_BUFFER_STATE ap_expr_yy_scan_bytes  (yyconst char * yybytes, int  _yybytes_len , yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n;
	int i;
	n = _yybytes_len + 2;
	buf = (char *) ap_expr_yyalloc(n ,yyscanner );
	for ( i = 0; i < _yybytes_len; ++i )
		buf[i] = yybytes[i];
	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
	b = ap_expr_yy_scan_buffer(buf,n ,yyscanner);
	if ( ! b )
		YY_FATAL_ERROR( "bad buffer in ap_expr_yy_scan_bytes()" );
	b->yy_is_our_buffer = 1;
	return b;
}
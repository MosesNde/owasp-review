#include <stdio.h>
#include <string.h>
#include "old_lexer_lib_v1.h"
YY_BUFFER_STATE ap_expr_yy_scan_bytes  (yyconst char * yybytes, int  _yybytes_len , yyscan_t yyscanner)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n;
	int i;
	n = (yy_size_t) (_yybytes_len + 2);
	buf = (char *) old_ap_expr_yyalloc(n ,yyscanner );
	if ( ! buf )
		old_YY_FATAL_ERROR( "out of dynamic memory in ap_expr_yy_scan_bytes()" );
	for ( i = 0; i < _yybytes_len; ++i )
		buf[i] = yybytes[i];
	buf[_yybytes_len] = buf[_yybytes_len+1] = OLD_YY_END_OF_BUFFER_CHAR;
	b = old_ap_expr_yy_scan_buffer(buf,n ,yyscanner);
	if ( ! b )
		old_YY_FATAL_ERROR( "bad buffer in ap_expr_yy_scan_bytes()" );
	b->yy_is_our_buffer = 1;
	return b;
}
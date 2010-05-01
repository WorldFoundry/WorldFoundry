%{
#undef YY_INPUT
#define YY_INPUT( b, r, ms )  ( r = my_yyinput( b, ms ) )

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "expr_tab.cc.h"           // internal only, I think
#include "expr.h"               // internal only, I think

long btol( const char *s );

extern char* _theExpression;
extern double (*_fnSymbolLookup)( const char* );

%}

%%

[0-9]+    {
	yylval.dval = atoi( yytext );
	return NUMBER;
	}

\$[0-9A-Fa-f]+     {
	long num;
	sscanf( yytext+1, "%x", &num );
	yylval.dval = num;
	return NUMBER;
	}

0[xX][0-9A-Fa-f]+  {
	long num;
	sscanf( yytext+2, "%x", &num );
	yylval.dval = num;
	return NUMBER;
	}

%[01\.]+	{
	yylval.dval = btol( yytext + 1 );
	return NUMBER;
	}

0[bB][01\.]+	{
	yylval.dval = btol( yytext + 2 );
	return NUMBER;
	}

-?[0-9]+\.?[0-9]*	{
	yylval.dval = atof( yytext );
	return NUMBER;
	}

[ \t] ; /* ignore whitespace */

[A-Za-z][A-Za-z0-9]* {	// return symbol pointer
//	struct symtab* sp = symlook( yytext );
//	yylval.symp = sp;

	yylval.dval = _fnSymbolLookup ? _fnSymbolLookup( yytext ) : 0.0;

	return NUMBER;
//	return NAME;
    }

"++" { return INC; }

"--" { return DEC; }

"<<" { return LSHIFT; }

">>" { return RSHIFT; }

"!"  { return LNOT; }

"~"  { return BNOT; }

"<"  { return LT; }

"<=" { return LTE; }

">"  { return GT; }

">=" { return GTE; }

"==" { return EQ; }

"="  { return ASSIGN; }

"!=" { return NE; }

"&&" { return LAND; }

"&"  { return BAND; }

"||" { return LOR; }

"|"  { return BOR; }

"^"  { return XOR; }

"+"  { return PLUS; }

"-"  { return MINUS; }

"*"  { return MULT; }

"/"  { return DIVIDE; }

"%"  { return MOD; }

, |
; |
\n  { return EOL; }

.    return yytext[ 0 ];

%%

long
btol( const char *_s )
{
	long value = 0;

	char* s = (char*)_s;
	while( *s )
	{
		assert( *s == '0' || *s == '1'  || *s == '.' );
		if( *s != '.' )
		{
			value <<= 1;
			value |= *s - '0';
		}
		s++;
	}
	return value;
}


int
yywrap( void )
{
	return 1;
}


void
yyerror( char* message )
{
	fprintf( stderr, message );
}

static int nTimes = 0;

void
yyreset()
{
	nTimes = 0;
}


int
my_yyinput( char* buf, int max_size )
{
	if ( nTimes )
		return 0;

	strcpy( buf, _theExpression );
	++nTimes;

	return( strlen( buf ) );
}

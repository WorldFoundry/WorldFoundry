%{
#include "expr.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

double theExpressionValue;
char* _theExpression;
double (*_fnSymbolLookup)( const char* );

// kts added 5/25/99 to build under linux
#include "unistd.h"
extern void yyerror( char* message );
extern int yylex( void );



%}

%union {
  double dval;
  struct symtab* symp;
}

%token <symp> NAME
%token <dval> NUMBER

%token EOL
%token LOR LAND EQ NE LT LTE GT GTE LSHIFT RSHIFT PLUS MINUS MULT DIVIDE MOD INC DEC ASSIGN LNOT BNOT

%right ASSIGN
%right '?' ':'
%left LOR
%left LAND
%left BOR
%left XOR
%left BAND
%left EQ NE
%left LT LTE GT GTE
%left LSHIFT RSHIFT
%left PLUS MINUS
%left MULT DIVIDE MOD
%left INC DEC
%nonassoc UMINUS LNOT BNOT

%type <dval> expression paren_expr
%%

statement :      expression					{ theExpressionValue = $1; }
  ;

paren_expr: '(' expression ')'				{ $$ = $2; }
  ;

expression: expression PLUS expression		{ $$ = $1 + $3; }
  |         expression MINUS expression		{ $$ = $1 - $3; }
  |         expression MULT expression		{ $$ = $1 * $3; }
  |         expression DIVIDE expression
  { if ( $3 == 0 )
	yyerror( "divide by zero" );
    else
	$$ = $1 / $3;
  }
/*  |         expression MOD expression
  { if ( $3 == 0 )
      yyerror( "divide by zero" );
    else
      $$ = int($1) % int($3);
  }
  */
  |         MINUS expression %prec UMINUS { $$ = -$2; }
  |         LNOT expression %prec LNOT    { $$ = !$2; }
/*  |         BNOT expression %prec BNOT    { $$ = ~$2; }*/
  |         INC expression %prec INC      { $$ = $2; ++$$; }
  |         expression INC %prec INC      { $$ = $1; $$++; }
  |         DEC expression %prec DEC      { $$ = $2; --$$; }
  |         expression DEC %prec DEC      { $$ = $1; $$--; }
/*  |         expression LSHIFT expression  { $$ = $1 << $3; }
  |         expression RSHIFT expression  { $$ = $1 >> $3; }
  */
  |         expression LT expression      { $$ = $1 < $3; }
  |         expression LTE expression     { $$ = $1 <= $3; }
  |         expression GT expression      { $$ = $1 > $3; }
  |         expression GTE expression     { $$ = $1 >= $3; }
  |         expression EQ expression      { $$ = $1 == $3; }
  |         expression NE expression      { $$ = $1 != $3; }
/*
  |         expression BAND expression    { $$ = $1 & $3; }
  |         expression XOR expression     { $$ = $1 ^ $3; }
  |         expression BOR expression     { $$ = $1 | $3; }
  */
  |         expression LAND expression    { $$ = $1 && $3; }
  |         expression LOR expression     { $$ = $1 || $3; }
  |         '?' expression                { printf( "%d\n", $2 ); $$ = $2; }           /**** TEMPORARY FOR TESTING *****/
  |         expression '?' expression ':' expression { $$ = ($1 ? $3 : $5); }
  |         NAME ASSIGN expression   { $1->value = $3; }
  |         paren_expr            { $$ = $1; }
  |         NUMBER
  |         NAME                          { $$ = $1->value; }
  |         NAME paren_expr		{
					      if ( $1->funcptr )
						  $$ = ($1->funcptr)($2);
					      else {
						  printf( "%s not a function\n", $1->name );
						  $$ = 0;
      }
  }
  ;

%%

// look up a symbol table entry, add if not present
struct symtab*
symlook( char* s )
{
        static struct symtab symtab[ NSYMS ];

  char* p;
  struct symtab* sp;

  for ( sp=symtab; sp < &symtab[ NSYMS ]; ++sp )
  {
      /* is it already known? */
      if ( sp->name && !strcmp( sp->name, s ) )
	  return sp;


      /* is it free? */
      if ( !sp->name )
      {
	  sp->name = strdup( s );
	  return sp;
      }

      /* otherwise continue to next */
  }
  yyerror( "Too many symbols" );
  exit( 1 );
        return NULL;    // to keep the compiler happy
}


void
addfunc( char* name, long (*func)(...) )
{
    struct symtab* sp = symlook( name );
    assert( sp );
    sp->funcptr = func;
}


long numberofones( long val )
{
	long ret_val = 0;

	for ( ; val; val >>= 1 )
  	{
  		if ( val & 1 )
    		++ret_val;
  	}
	return ret_val;
}


void
define_func( char* s1, char* s2 )
{
}


void yyreset();

double
eval( const char* szExpression, double (*fnSymbolLookup)( const char* szSymbolName ) )
{
	assert( szExpression );
        _theExpression = (char*)szExpression;

	//assert( fnSymbolLookup );
	_fnSymbolLookup = fnSymbolLookup;

	theExpressionValue = 0.0;
	yyparse();
	yyreset();
	return theExpressionValue;
}

%{
//==============================================================================
// lang.l: Copyright (c) 1996-1999, World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org
//==============================================================================

#include <cassert>
#include <cstring>
#include <stdio.h>
class istream;
#include <strstream>
#include <iomanip>
using namespace std;
#include <pigsys/pigsys.hp>
#include <iffwrite/fixed.hp>
#include "grammar.hpp"
#include "lang_tab.h"
#include "langlex.hpp"
extern strFlexLexer* theLexer;

typedef struct
{
	YY_BUFFER_STATE yy_buffer_state;
	unsigned long nLine;
	char szFilename[ PATH_MAX ];
} INCLUDE_STATE;


int
checkSize( const char* str )
{
	int sizeOverride = 0;

	char ch = *( str + strlen( str ) - 1 );
	if ( ch == 'y' )
		sizeOverride = 1;
	else if ( ch == 'w' )
		sizeOverride = 2;
	else if ( ch == 'l' )
		sizeOverride = 4;

	return sizeOverride;
}


int
checkSizeReal( const char* str )
{
	int sizeOverride = 0;

	char ch = *( str + strlen( str ) - 1 );
	if ( ch == 'w' )
		sizeOverride = 2;
	else if ( ch == 'l' )
		sizeOverride = 4;
//	else if ( ch == 'y' )
//		sizeOverride = 1;

	return sizeOverride;
}


size_specifier
parse_size_specifier( const char* szText )
{
	size_specifier ret;

	sscanf( szText, "%d.%d.%d", &ret.sign, &ret.whole, &ret.fraction );
	if ( ( ret.sign < 0 ) || ( ret.sign > 1 ) )
	{
		theGrammar->Warning( "Sign must be either 0 or 1 [assuming 1]" );
		ret.sign = 1;
	}

	return ret;
}

//	\"[^\"\n]*[\"\n]

%}

%x INCLUDE

%%

include[ \t]+\"					{ BEGIN INCLUDE; }

<INCLUDE>[^\">]*		   		{
	int chClosingQuote = yyinput();			// read closing quote
	if ( chClosingQuote == '>' )
		theLexer->push_system_include( YYText() );
	else
		theLexer->push_include( YYText() );
	BEGIN INITIAL;
	}

include[ \t]+<				{ BEGIN INCLUDE; }

\.timestamp				{ return TIMESTAMP; }

\.align					{ return ALIGN; }

\.offsetof				{ return OFFSETOF; }

\.sizeof				{ return SIZEOF; }

\.fillchar				{ return FILLCHAR; }

\.start					{ return START; }

\.length				{ return LENGTH; }

\.precision				{ return PRECISION; }

::					{ return DOUBLE_COLON; }

"+"					{ return PLUS; }

"-"					{ return MINUS; }

'.{1,4}'				{
						int shift = 24;
						unsigned long l = 0;
						int lenID = strlen( YYText() ) - 2;
						for ( int i=0; i<4; ++i )
						{
							unsigned long n = i < lenID ? *( YYText() + 1 + i ) : 0;
							l |= n << shift;
							shift -= 8;
						}
						yylval.integer.val = l;

						//printf( "char_literal = %08x\n", l );
						return CHAR_LITERAL;
					}

[0-9]+\.[0-9]+\.[0-9]+	{
//	printf( "found precision specifier [%s]\n", YYText() );
	yylval.size_specifier = parse_size_specifier( YYText() );
	return PRECISION_SPECIFIER;
}



\$[0-9A-Fa-f]+[ywl]?	{
	sscanf( YYText()+1, "%lx", &yylval.integer.val );
	yylval.integer.sizeOverride = checkSize( YYText() );
	//printf( "INTEGER: [0x%lx] [%d]\n", yylval.integer.val, yylval.integer.sizeOverride );
	return INTEGER;
}


-?[0-9]+[ywl]?		{
//	istrstream buf( (char*)YYText(), strlen( YYText() ) );
// 	buf >> yylval.val;
	sscanf( YYText(), "%ld", &yylval.integer.val );
	yylval.integer.sizeOverride = checkSize( YYText() );
	//printf( "INTEGER: [%ld] [%d]\n", yylval.integer.val, yylval.integer.sizeOverride );
	return INTEGER;
}


-?(([0-9]*(\.[0-9]+)([eE][+-]?[0-9]+)?)|([0-9]+)){1}(\([0-9]+\.[0-9]+\.[0-9]+\))?	{
	// handles #.#, #.#(), .#, .#(), #()
	istrstream buf( (char*)YYText(), strlen( YYText() ) );
 	buf >> yylval.real.dval;
	char* szPrecisionOverride = strchr( YYText(), '(' );
	yylval.real.size_specifier = szPrecisionOverride
		? parse_size_specifier( szPrecisionOverride + 1 )
		: theGrammar->vecState.back().precision();
	return REAL;
}


\"(\\\"|\\\\|[^\"\n])*[\"\n] {
	char* szEndOfString;

	//printf( "STRING = [%s]\n", YYText() );

	yylval.string.str = strdup( YYText()+1 );
	assert( yylval.string.str );
	szEndOfString = yylval.string.str + strlen( yylval.string.str ) - 1;
	if ( *szEndOfString != '"' )
		theGrammar->Warning( "Unterminated string (%s)\n", yylval.string.str );
	else
		*szEndOfString= '\0';
	// or should it be -1? what is a 0-length string?  
	// especially one that's null terminated!?
	yylval.string.sizeOverride = 0;		

//	printf( "string=[%s], after string=[%s]\n", yylval.string.str, szEndOfString+1 );
	return STRING;
}

\"(\\\"|\\\\|[^\"\n])*[\"\n](\([0-9]+\))? {
	char* szEndOfString;

//	printf( "STRING (with size specifier) = [%s]\n", YYText() );

	yylval.string.str = strdup( YYText()+1 );
	assert( yylval.string.str );
	// we are guaranteed that the string is terminated (") otherwise
	// it wouldn't have matched this rule [i think]
	szEndOfString = rindex( yylval.string.str, '"' );
	*szEndOfString= '\0';
	yylval.string.sizeOverride = atoi( szEndOfString+2 );

//	printf( "string=[%s], after string=[%s], sizeOverride=[%d]\n", yylval.string.str, szEndOfString+1, yylval.string.sizeOverride );
	return STRING;
}

\/\/.*$							{ }

\n.+$	   						{
							//cout << '[' << yytext+1 << ']' << endl;
							assert( theLexer );
							theLexer->currentLine( YYText()+1 );
							theLexer->nextLine();
							yyless( 1 );
							}

\n$  							{ /*printf( "new line\n" );*/
     							theLexer->nextLine();
     							}

[\t ]+	 						{ /* ignore whitespace */ }

<<EOF>>							{ if ( !theLexer->pop_include() ) yyterminate(); }

.							{ return *YYText(); }

%%

void
yyerror( char* message )
{
	assert( theGrammar );
	theGrammar->Error( message );
}

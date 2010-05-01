%{

//==============================================================================
// c.l: Copyright (c) 1996-1999, World Foundry Group  
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

#include <stdio.h>
#include <pigsys/assert.hp>
#include <string.h>
#include <fstream>

#include "symbol.hp"
#include "obj/tokens_tab.h"
#ifndef _WIN32
#include "FlexLexer.h"
#else
#include "flexle~1.h"
#endif
#include "grammar.hp"

typedef struct
{
	YY_BUFFER_STATE yy_buffer_state;
	int32 nLine;
	char szFilename[ _MAX_PATH ];
} INCLUDE_STATE;

#include "ailexer.hp"

extern aiFlexLexer* theLexer;

#define RETURN_ID( __id__ )			return ( yylval.opcode = ( __id__ ) )

%}

%x INCLUDE

%%


include[ \t]+\"					{ BEGIN INCLUDE; }

<INCLUDE>[^\">]*		   		{
	int chClosingQuote = yyinput();	// Read closing quote
	if ( chClosingQuote == '>' )
		theLexer->push_system_include( YYText() );
	else
		theLexer->push_include( YYText() );
	BEGIN INITIAL;
	}

include[ \t]+<					{ BEGIN INCLUDE; }

"false"						{ yylval.val = Scalar::zero; RETURN_ID( NUMBER ); }

"true" 						{ yylval.val = Scalar::one; RETURN_ID( NUMBER ); }

define	      					{ RETURN_ID( DEFINE ); }

if    						{ RETURN_ID( IF ); }

else  						{ RETURN_ID( ELSE ); }

branch	      					{ RETURN_ID( BRANCH ); }

random						{ RETURN_ID( RANDOM ); }

find-class    					{ RETURN_ID( FIND_CLASS ); }

create-object					{ RETURN_ID( CREATE_OBJECT ); }

sleep	      					{ RETURN_ID( SLEEP ); }

euler-to-vector					{ RETURN_ID( EULER_TO_VECTOR ); }

vector-to-euler					{ RETURN_ID( VECTOR_TO_EULER ); }

exit	       					{ RETURN_ID( EXIT ); }

event	       					{ RETURN_ID( EVENT ); }

mailbox	       					{ RETURN_ID( MAILBOX ); }


sin	      					{ RETURN_ID( SIN ); }

cos	      					{ RETURN_ID( COS ); }

asin	      					{ RETURN_ID( ASIN ); }

acos	      					{ RETURN_ID( ACOS ); }

tan	      					{ RETURN_ID( TAN ); }

atan2	      					{ RETURN_ID( ATAN2 ); }

atan2quick    					{ RETURN_ID( ATAN2QUICK ); }

"<"   						{ RETURN_ID( LT ); }

"<="  						{ RETURN_ID( LTE ); }

">"   						{ RETURN_ID( GT ); }

">="  						{ RETURN_ID( GTE ); }

"="	      					{ RETURN_ID( EQ ); }

"!="	      					{ RETURN_ID( NE ); }

"+"	      					{ RETURN_ID( PLUS ); }

"-"	      					{ RETURN_ID( MINUS ); }

"*"	      					{ RETURN_ID( MULT ); }

"/"	      					{ RETURN_ID( DIVIDE ); }

min	      					{ RETURN_ID( OP_MIN ); }

max	      					{ RETURN_ID( OP_MAX ); }

round	      					{ RETURN_ID( ROUND ); }

floor	      					{ RETURN_ID( FLOOR ); }

ceiling	      					{ RETURN_ID( CEILING ); }

abs	      					{ RETURN_ID( OP_ABS ); }

truncate      					{ RETURN_ID( TRUNCATE ); }

remainder     					{ RETURN_ID( REMAINDER ); }

modulo	      					{ RETURN_ID( QUOTIENT ); }

and	      					{ RETURN_ID( LAND ); }

or	      					{ RETURN_ID( LOR ); }

not	      					{ RETURN_ID( LNOT ); }

inc	      					{ RETURN_ID( INC ); }

dec	      					{ RETURN_ID( DEC ); }

"<<"	      					{ RETURN_ID( LSHIFT ); }

">>"	      					{ RETURN_ID( RSHIFT ); }

"&"	      					{ RETURN_ID( BAND ); }

"|"	      					{ RETURN_ID( BOR ); }

"~"	      					{ RETURN_ID( BNOT ); }

"^"	      					{ RETURN_ID( XOR ); }


#'.{1,4}'					{
							int shift=0;
							unsigned long l = 0;
							//assert( (0 >= yyleng-2-1) && (yyleng-2-1 <= 4) );
							for ( int i=0; i<yyleng-2-1; ++i )
							{
								l |= *( YYText()+2+i ) << shift;
								shift += 8;
							}
							yylval.l = l;
							RETURN_ID( CHAR_LITERAL );
						}

-?[0-9]+\.?[0-9]*				{
	yylval.val = SCALAR_CONSTANT( atof( YYText() ) );
	//cout << YYText() << " = " /*<< f << " = "*/ << yylval.val << endl;
	//AssertMsg( nItems == 1, "nItems = " << nItems );
	RETURN_ID( NUMBER );
	}


\"[^\"\n]*[\"\n]				{
	char* szEndOfString;

	yylval.name = strdup( YYText()+1 );
	assert( yylval.name );
	szEndOfString = yylval.name + strlen( yylval.name ) - 1;
	if ( *szEndOfString != '"' )
		theGrammar->Warning( "Unterminated string (%s)\n", yylval.name );
//		fprintf( stderr, "Warning: Unterminated string (%s)\n", yylval.name );
	else
		*szEndOfString= '\0';

	RETURN_ID( STRING );
	}

	/*		 ? ! . + - * / < = > : $ % ^ & _ ~	*/

	/*	[A-Za-z_-]+[A-Za-z0-9_-]*		{	*/

[A-Za-z0-9_-]+		{
	//printf( "Object name: [%s]\n", YYText() );
	yylval.name = strdup( YYText() );
	assert( yylval.name );
	if ( theGrammar->find_mailbox( yylval.name ) )
		RETURN_ID( MAILBOX_TYPE );
	else
		RETURN_ID( ACTOR );
	}


\/\/.*$							{ /*theLexer->nextLine();*/ }

\n.+$	   						{
								//cout << '[' << yytext+1 << ']' << endl;
								assert( theLexer );
								theLexer->currentLine( yytext+1 );
								theLexer->nextLine();
								yyless( 1 );
							}

\n$							{ //printf( "new line\n" );
								theLexer->nextLine();
							}

[\t ]+	 						{ /* ignore whitespace */ }

<<EOF>>							{ if ( !theLexer->pop_include() ) yyterminate(); }

.							{ yylval.opcode = 0; return *YYText(); }

%%

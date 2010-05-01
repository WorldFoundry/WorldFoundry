%{
//
//==============================================================================
// c.y: Copyright (c) 1996-1999, World Foundry Group  
// AI Compiler Grammar [C style]
// By William B. Norris IV
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

#include <string.h>
#include <pigsys/assert.hp>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdarg.h>

#include <string>
#include <vector>

#include <iffwrite/id.hp>

#include "symbol.hp"
#ifndef _WIN32
#include "FlexLexer.h"
#else
#include "flexle~1.h"
#endif
#include "airun.hp"
#include "fileline.hp"
#include "ailexer.hp"

extern int programReturnCode;

extern char szVersion[];

extern bool bVerbose;	// = true;			// Verbose output
extern bool bQuiet;	// = false;			// No error output

extern aiFlexLexer* theLexer;

#include "grammar.hp"

void beep();
int yyparse();

extern aiGrammar* theGrammar;

#define OPCODE( __label__ )		theGrammar->out_opcode( $1, __label__ )

#include <math/scalar.hp>

%}

%union
{
	struct
	{
		//float f;
		char* name;
		Scalar val;
		unsigned long l;
		unsigned long opcode;
	} data;
}


%token BASE_TOKEN_VAL

%token <data.opcode> LT LTE GT GTE EQ NE
%token <data.opcode> MULT DIVIDE PLUS MINUS QUOTIENT REMAINDER NEGATE
%token <data.opcode> LAND LOR LNOT INC DEC LSHIFT RSHIFT BAND BOR BNOT XOR
%token <data.opcode> IF COND ELSE IF_THEN IF_THEN_ELSE
%token <data.opcode> ZERO_Q POSITIVE_Q NEGATIVE_Q ROUND TRUNCATE FLOOR CEILING OP_ABS
%token <data.opcode> OP_MIN OP_MAX
%token <data.opcode> NEWLINE WRITE WRITELN
%token <data.opcode> READ_MAILBOX WRITE_TO_MAILBOX SEND_MESSAGE RANDOM
%token <data.name> ACTOR MAILBOX_TYPE
%token <data.name> STRING
%token <data.scalarhiddeninlong> NUMBER
%token <data.opcode> DEFINE LAMBDA FIND_CLASS CREATE_OBJECT SELF SLEEP SET
%token <data.opcode> SIN COS ASIN ACOS ATAN2 ATAN2QUICK TAN
%token <data.opcode> BEGINP EULER_TO_VECTOR VECTOR_TO_EULER EXIT
%token <data.opcode> CHAR_LITERAL MEMBER EVENT MAILBOX
%token <data.opcode> BRANCH			/* internal code only */

%type <data.scalarhiddeninlong> expr

%%

script :	'{' statement_list '}'
	|								{ theGrammar->out_number( Scalar::zero ); }
	;

statement_list :	statement
	|		statement_list statement
	;

statement :	'{'							{ theGrammar->parseLog() << "<tr>\n\t<td bgcolor=\"#808000\"><font color=\"white\">" <<
										"<code>" << theLexer->currentLine() << "</code></font>";
									}
			core_statement
		'}'							{ theGrammar->parseLog() << "</td>\n</tr>" << endl; }
	;

core_statement :
			expr						{ theGrammar->next_line(); }
	|		defun						{}
	|		event                       			{ theGrammar->next_line(); }
	;


event :
			EVENT ACTOR expr_list		{}
	;


defun :
			DEFINE ACTOR '(' LAMBDA ACTOR
		{
			theGrammar->def_lambda( $2, $5 );
			theGrammar->in_def( true );
		}
			expr_list ')'
		{
			theGrammar->in_def( false );
		}
	;

expr_list :	'(' expr ')'
	|	expr_list '(' expr ')'
	;



condition :
			expr	// condition
		{
			theGrammar->out_opcode( BRANCH /*, "branch"*/ );
			// mark backpatch destination (else)
			Backpatch bp( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.push_back( bp );
			theGrammar->out_number( Scalar::zero );
			theGrammar->end_opcode();
		}
	;


consequent :
			expr	// consequent
		{
			{ // mark backpatch destination (endif)
			theGrammar->out_opcode( BRANCH /*, "branch"*/ );
			Backpatch bp( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.push_back( bp );
			theGrammar->out_number( Scalar::zero );
			theGrammar->end_opcode();
			}

			{ // set backpatch destination (else)
			assert( theGrammar->_tblBackpatch.size() >= 2 );
			Backpatch bp = theGrammar->_tblBackpatch[ theGrammar->_tblBackpatch.size()-1-1 ];
			bp.backpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			}
		}
	;


alternative :	/* empty */
		{
			// set backpatch destination (endif)
			Backpatch bp = theGrammar->_tblBackpatch.back();
			bp.backpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.pop_back();	// else
			theGrammar->_tblBackpatch.pop_back();	// endif
		}
	|	expr	// alternative
		{
			//bVerbose && printf( "Writing alternative\n" );
			// set backpatch destination (endif)
			Backpatch bp = theGrammar->_tblBackpatch.back();
			bp.backpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.pop_back();	// else
			theGrammar->_tblBackpatch.pop_back();	// endif
		}
	;


expr :
			DEFINE ACTOR NUMBER
		{
			theGrammar->add_symbol( $2, $3 );
			theGrammar->_nTokensInProgram -= theGrammar->nTokensInLine;
			theGrammar->nTokensInLine = 0;
		}
	|		DEFINE ACTOR ACTOR
		{
			Symbol* symbol = theGrammar->find_symbol( $3 );
			if ( symbol )
				theGrammar->add_symbol( $2, symbol->nSymbolValue );
			else
				theGrammar->Error( "Undefined symbol \"%s\"", $3 );

			theGrammar->_nTokensInProgram -= theGrammar->nTokensInLine;
			theGrammar->nTokensInLine = 0;
		}
	|		DEFINE ACTOR statement
		{
		}
	|		ACTOR
		{
			theGrammar->out_actor( $1 );
		}
	|		NUMBER
		{
			//cout << "NUMBER = " << $1 << endl;
			theGrammar->parseLog() << "<font color=\"purple\">" << $1 << "</font> ";	//"out_number( " << $1/65536.0 << " )";
			theGrammar->out_number( $1 );
		}
	|		PLUS { theGrammar->out_opcode( $1, "+" ); } expr expr { theGrammar->end_opcode(); }
	|		MINUS { theGrammar->out_opcode( $1, "-" ); } expr expr { theGrammar->end_opcode(); }
	|		MULT { theGrammar->out_opcode( $1, "*" ); } expr expr { theGrammar->end_opcode(); }
	|		DIVIDE { theGrammar->out_opcode( $1, "/" ); } expr expr { theGrammar->end_opcode(); }
		{
			if ( $3 == Scalar::zero )
				theGrammar->Error( "divide by zero" );
		}
	|		QUOTIENT { theGrammar->out_opcode( $1, "quotient" ); } expr expr { theGrammar->end_opcode(); }
		{
			if ( $3 == Scalar::zero )
				theGrammar->Error( "divide by zero" );
		}
	|		REMAINDER { theGrammar->out_opcode( $1, "remainder" ); } expr expr { theGrammar->end_opcode(); }
		{
			if ( $3 == Scalar::zero )
				theGrammar->Error( "divide by zero" );
		}
	|		LT { theGrammar->out_opcode( $1, "<" ); } expr expr { theGrammar->end_opcode(); }
	|		LTE { theGrammar->out_opcode( $1, "<=" ); } expr expr { theGrammar->end_opcode(); }
	|		GT { theGrammar->out_opcode( $1, ">" ); } expr expr { theGrammar->end_opcode(); }
	|		GTE { theGrammar->out_opcode( $1, ">=" ); } expr expr { theGrammar->end_opcode(); }
	|		EQ { theGrammar->out_opcode( $1, "=" ); } expr expr { theGrammar->end_opcode(); }
	|		NE { theGrammar->out_opcode( $1, "!=" ); } expr expr { theGrammar->end_opcode(); }
	|		LAND { theGrammar->out_opcode( $1, "and" ); } expr expr { theGrammar->end_opcode(); }
	|		LOR { theGrammar->out_opcode( $1, "or" ); } expr expr { theGrammar->end_opcode(); }
	|		LNOT { theGrammar->out_opcode( $1, "not" ); } expr { theGrammar->end_opcode(); }
	|		'(' expr ')'
		{
			//if ( bVerbose ) cout << "$$ = " << $2 << endl;
			$$ = $2;
		}
	|		IF { theGrammar->out_opcode( IF_THEN_ELSE, "if (then-else)" ); }
			condition
			consequent
			ELSE
			alternative
			{ theGrammar->end_opcode(); }
	|		BEGINP
		{
			//theGrammar->out_opcode( $1, "begin" );
			Backpatch bp( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.push_back( bp );
			theGrammar->out_number( Scalar::zero );
			//theGrammar->end_opcode();
		}
			expr_list
		{
			Backpatch bp = theGrammar->_tblBackpatch.back();
			bp.backpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.pop_back();
		}
	|		DEFINE MAILBOX ACTOR NUMBER
		{
			theGrammar->add_mailbox( $3, $4 );
			theGrammar->_nTokensInProgram -= theGrammar->nTokensInLine;
			theGrammar->nTokensInLine = 0;
		}
	|       	ACTOR '.' MAILBOX_TYPE
		{
			theGrammar->out_opcode( READ_MAILBOX, "read-mailbox" );
			theGrammar->out_actor( $1 );
			theGrammar->out_actor( $3 );
			theGrammar->end_opcode();
		}
	|		MAILBOX_TYPE
		{
			theGrammar->out_opcode( READ_MAILBOX, "read-mailbox" );
			theGrammar->out_actor( "self" );
			theGrammar->out_actor( $1 );
			theGrammar->end_opcode();
		}
	|		ACTOR '.' MAILBOX_TYPE EQ
		{
			theGrammar->out_opcode( WRITE_TO_MAILBOX, "write-mailbox" );
			theGrammar->out_actor( $1 );
			theGrammar->out_actor( $3 );
			theGrammar->end_opcode();
		}
			expr	//NUMBER
	|		MAILBOX_TYPE EQ
		{
			theGrammar->out_opcode( WRITE_TO_MAILBOX, "write-mailbox" );
			theGrammar->out_actor( "self" );
			theGrammar->out_actor( $1 );
			theGrammar->end_opcode();
		}
			expr	//NUMBER
	|		SEND_MESSAGE { theGrammar->out_opcode( $1, "send-message" ); } expr expr expr { theGrammar->end_opcode(); }
	|		INC { theGrammar->out_opcode( $1, "++" ); } expr { theGrammar->end_opcode(); }
	|		DEC { theGrammar->out_opcode( $1, "--" ); } expr { theGrammar->end_opcode(); }
	|		LSHIFT { theGrammar->out_opcode( $1, "<<" ); } expr expr { theGrammar->end_opcode(); }
	|		RSHIFT { theGrammar->out_opcode( $1, ">>" ); } expr expr { theGrammar->end_opcode(); }
	|		BAND { theGrammar->out_opcode( $1, "&" ); } expr expr { theGrammar->end_opcode(); }
	|		BOR { theGrammar->out_opcode( $1, "|" ); } expr expr { theGrammar->end_opcode(); }
	|		BNOT { theGrammar->out_opcode( $1, "~" ); } expr { theGrammar->end_opcode(); }
	|		XOR { theGrammar->out_opcode( $1, "^" ); } expr expr { theGrammar->end_opcode(); }
	|		RANDOM { theGrammar->out_opcode( $1, "random" ); } expr { theGrammar->end_opcode(); }
	|		ROUND { theGrammar->out_opcode( $1, "round" ); } expr { theGrammar->end_opcode(); }
	|		TRUNCATE { theGrammar->out_opcode( $1, "truncate" ); } expr { theGrammar->end_opcode(); }
	|		FLOOR { theGrammar->out_opcode( $1, "floor" ); } expr { theGrammar->end_opcode(); }
	|		CEILING	{ theGrammar->out_opcode( $1, "ceiling" ); } expr { theGrammar->end_opcode(); }
	|		OP_ABS { theGrammar->out_opcode( $1, "abs" ); } expr { theGrammar->end_opcode(); }
	|		OP_MIN { theGrammar->out_opcode( $1, "min" ); } expr expr { theGrammar->end_opcode(); }
	|		OP_MAX { theGrammar->out_opcode( $1, "max" ); } expr expr { theGrammar->end_opcode(); }
	|		SIN { theGrammar->out_opcode( $1, "sin" ); } expr { theGrammar->end_opcode(); }
	|		COS { theGrammar->out_opcode( $1, "cos" ); } expr { theGrammar->end_opcode(); }
	|		TAN { theGrammar->out_opcode( $1, "tan" ); } expr { theGrammar->end_opcode(); }
	|		ASIN { theGrammar->out_opcode( $1, "asin" ); } expr { theGrammar->end_opcode(); }
	|		ACOS { theGrammar->out_opcode( $1, "acos" ); } expr { theGrammar->end_opcode(); }
	|		ATAN2 { theGrammar->out_opcode( $1, "atan2" ); } expr expr { theGrammar->end_opcode(); }
	|		ATAN2QUICK { theGrammar->out_opcode( $1, "atan2quick" ); } expr expr { theGrammar->end_opcode(); }
	|		CREATE_OBJECT { theGrammar->out_opcode( $1, "create-object" ); } expr expr expr expr { theGrammar->end_opcode(); }
	|		FIND_CLASS { theGrammar->out_opcode( $1, "find-class" ); } expr { theGrammar->end_opcode(); }
	|		SLEEP { theGrammar->out_opcode( $1, "sleep" ); } expr { theGrammar->end_opcode(); }
	|		BRANCH { theGrammar->out_opcode( $1/*, "(branch)"*/ ); } expr { theGrammar->end_opcode(); }
	|		EULER_TO_VECTOR { theGrammar->out_opcode( $1, "spherical-to-cartesian" ); }
				expr expr expr expr expr expr
				{ theGrammar->end_opcode(); }
	|		VECTOR_TO_EULER { theGrammar->out_opcode( $1, "cartesian-to-spherical" ); }
				expr expr expr expr expr
				 { theGrammar->end_opcode(); }
	|		EXIT { theGrammar->out_opcode( $1, "exit" ); theGrammar->end_opcode(); }
	|		CHAR_LITERAL
		{
			theGrammar->out_opcode( $1 );	//, "(char-literal)" );
			theGrammar->out_int32( yylval.l );
			ID id( yylval.l );
			//ID id( 'test' );
			theGrammar->parseLog() << id;
			theGrammar->end_opcode();
		}
	|		ACTOR '.' ACTOR
		{
			theGrammar->out_opcode( MEMBER, "(member)" );
			theGrammar->out_actor( $1 );
			theGrammar->out_actor( $3 );
			theGrammar->end_opcode();
		}
	;


string :
			STRING							{ theGrammar->out_string( $1 ); }
	;


%%

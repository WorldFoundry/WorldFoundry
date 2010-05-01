%{
//==============================================================================
// scheme.y: Copyright (c) 1996-1999, World Foundry Group  
// AI Compiler Grammar [Scheme style]
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

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <stdarg.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <iffwrite/id.hp>
#include <game/ai_tab.h>
#include "symbol.hp"
#ifndef _WIN32
#include "FlexLexer.h"
#else
#include "flexle~1.h"
#endif
#include "airun.hp"
#include "fileline.hp"
#include "ailexer.hp"
//#include <scheme/ai_tab.h>
#include "flex.h"
extern int programReturnCode;

extern char szVersion[];

extern bool bVerbose;	// = true;			// Verbose output
extern bool bQuiet;	// = false;			// No error output

extern aiFlexLexer* theLexer;

#include "grammar.hp"

void beep();
int yyparse();

extern aiGrammar* theGrammar;

//#define OPCODE( __label__ )		theGrammar->out_opcode( $1, __label__ )

#include <math/scalar.hp>

%}

%union
{
	struct
	{
		//float f;
		char* name;
		//Scalar val; 
		unsigned long scalarhiddeninlong;
		unsigned long l;
		unsigned long opcode;
	} data;
}

%token BASE_TOKEN_VAL

%token <data.opcode> LT LTE GT GTE EQ NE
%token <data.opcode> MULT DIVIDE PLUS MINUS QUOTIENT REMAINDER NEGATE
%token <data.opcode> LAND LOR LNOT INC DEC LSHIFT RSHIFT BAND BOR BNOT XOR
%token <data.opcode> IF COND ELSE
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
%token <data.opcode> PTR_DEREF
%type <data.scalarhiddeninlong> expr

%%

script :	'('
									{
										string szMain( "main" );
										Function* fn = new Function( szMain );
										assert( fn );
										theGrammar->_tblFunction.push_back( fn );
										Function* s = theGrammar->_tblFunction.back();
//										assert( s );
//										theGrammar->_tblFunction.pop_back();
//										theGrammar->commit( s );
										// grammar deletes these after it writes them out
									}
			statement_list
		')'
	|								{
										string szMain( "main" );
										Function* fn = new Function( szMain );
										assert( fn );
										theGrammar->_tblFunction.push_back( fn );
										theGrammar->function()->out_opcode( AI_EXIT );
										Function* s = theGrammar->_tblFunction.back();
//										assert( s );
//										theGrammar->_tblFunction.pop_back();
//										theGrammar->commit( s );
										// grammar deletes these after it writes them out
									}
	;

statement_list :	statement
	|		statement_list statement
	;

statement :	'('							{ theGrammar->parseLog() << "<tr>\n\t<td bgcolor=\"#808000\"><font color=\"white\">" <<
										"<code>" << theLexer->currentLine() << "</code></font>";
//									string szFunction( "test" );
//									theGrammar->_tblFunction.push_back( new Function( szFunction ) );
									}
			core_statement
		')'							{ theGrammar->parseLog() << "<! close ) --></td>\n</tr>" << endl;
//									Function* s = theGrammar->_tblFunction.back();
//									assert( s );
//									theGrammar->_tblFunction.pop_back();
//									theGrammar->commit( s );
									// grammar deletes these after it writes them out
									}
	;

core_statement :
			expr						{}
	|		defun                                           {}
	|		event                                           {}
	;


event :
			EVENT ACTOR expr_list		{}
	;


defun :
			DEFINE ACTOR					{
										theGrammar->_tblFunction.push_back( new Function( string( $2 ) ) );
//										theGrammar->function()->out_opcode( AI_EXIT );
									}
				'(' LAMBDA '(' ACTOR ')'
					expr_list
				')'
									{
										Function* s = theGrammar->_tblFunction.back();
										assert( s );
//										theGrammar->_tblFunction.pop_back();
										theGrammar->commit( s );
									}
	;


expr_list :	'(' expr ')'
	|	expr_list '(' expr ')'
	;


condition :
			expr	// condition
		{
			theGrammar->out_opcode( AI_BRANCH /*, "branch"*/ );
			// mark backpatch destination (else)
			theGrammar->_tblBackpatch.push_back( Backpatch( theGrammar->function() ) );
			theGrammar->function()->out_number( Scalar::negativeOne );	// placeholder for distance
#if 0
//			Backpatch bp( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
//			theGrammar->_tblBackpatch.push_back( bp );
#endif
			theGrammar->end_opcode();
		}
	;


consequent :
			expr	// consequent
		{
			{ // mark backpatch destination (endif)
			theGrammar->out_opcode( AI_BRANCH /*, "branch"*/ );
			theGrammar->_tblBackpatch.push_back( Backpatch( theGrammar->function() ) );
			theGrammar->function()->out_number( Scalar::negativeOne );
#if 0
//			Backpatch bp( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
//			theGrammar->_tblBackpatch.push_back( bp );
#endif
			theGrammar->end_opcode();
			}

			{ // set backpatch destination (else)
			assert( theGrammar->_tblBackpatch.size() >= 2 );
			Backpatch bp = theGrammar->_tblBackpatch[ theGrammar->_tblBackpatch.size()-1-1 ];
			bp.doBackpatch( theGrammar->function() );
#if 0
//			Backpatch bp = theGrammar->_tblBackpatch[ theGrammar->_tblBackpatch.size()-1-1 ];
//			bp.doBackpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
#endif
			}
		}
	;


alternative :	/* empty */
		{
			// set backpatch destination (endif)
			Backpatch bp = theGrammar->_tblBackpatch.back();
			bp.doBackpatch( theGrammar->function() );
//			bp.doBackpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.pop_back();	// else
			theGrammar->_tblBackpatch.pop_back();	// endif
		}
	|	expr	// alternative
		{
			// set backpatch destination (endif)
			Backpatch bp = theGrammar->_tblBackpatch.back();
			bp.doBackpatch( theGrammar->function() );
//			bp.doBackpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.pop_back();	// else
			theGrammar->_tblBackpatch.pop_back();	// endif
		}
	;


expr :
			DEFINE ACTOR NUMBER
		{
			theGrammar->add_symbol( $2, Scalar($3) );
		}
	|		DEFINE ACTOR ACTOR
		{
			Symbol* symbol = theGrammar->find_symbol( $3 );
			if ( symbol )
				theGrammar->add_symbol( $2, symbol->nSymbolValue );
			else
				theGrammar->Error( "Undefined symbol \"%s\"", $3 );
		}
/*	|		DEFINE ACTOR statement
		{
		}
*/
	|		ACTOR
		{
			theGrammar->out_actor( $1 );
		}
	|		NUMBER
		{
			theGrammar->parseLog() << "<font color=\"purple\">" << $1 << "</font> ";
			theGrammar->out_number( Scalar($1) );
		}
	|		PLUS { theGrammar->out_opcode( AI_PLUS, "+" ); } expr expr { theGrammar->end_opcode(); }
	|		MINUS { theGrammar->out_opcode( AI_MINUS, "-" ); } expr expr { theGrammar->end_opcode(); }
	|		MULT { theGrammar->out_opcode( AI_MULT, "*" ); } expr expr { theGrammar->end_opcode(); }
	|		DIVIDE { theGrammar->out_opcode( AI_DIVIDE, "/" ); } expr expr { theGrammar->end_opcode(); }
		{
			if ( $3 == Scalar::zero )
				theGrammar->Error( "divide by zero" );
		}
	|		QUOTIENT { theGrammar->out_opcode( AI_QUOTIENT, "quotient" ); } expr expr { theGrammar->end_opcode(); }
		{
			if ( $3 == Scalar::zero )
				theGrammar->Error( "divide by zero" );
		}
	|		REMAINDER { theGrammar->out_opcode( AI_REMAINDER, "remainder" ); } expr expr { theGrammar->end_opcode(); }
		{
			if ( $3 == Scalar::zero )
				theGrammar->Error( "divide by zero" );
		}
	|		LT   { theGrammar->out_opcode( AI_LT  , "<" ); } expr expr { theGrammar->end_opcode(); }
	|		LTE  { theGrammar->out_opcode( AI_LTE , "<=" ); } expr expr { theGrammar->end_opcode(); }
	|		GT   { theGrammar->out_opcode( AI_GT  , ">" ); } expr expr { theGrammar->end_opcode(); }
	|		GTE  { theGrammar->out_opcode( AI_GTE , ">=" ); } expr expr { theGrammar->end_opcode(); }
	|		EQ   { theGrammar->out_opcode( AI_EQ  , "=" ); } expr expr { theGrammar->end_opcode(); }
	|		NE   { theGrammar->out_opcode( AI_NE  , "!=" ); } expr expr { theGrammar->end_opcode(); }
	|		LAND { theGrammar->out_opcode( AI_LAND, "and" ); } expr expr { theGrammar->end_opcode(); }
	|		LOR  { theGrammar->out_opcode( AI_LOR , "or" ); } expr expr { theGrammar->end_opcode(); }
	|		LNOT { theGrammar->out_opcode( AI_LNOT, "not" ); } expr { theGrammar->end_opcode(); }
	|		'(' expr ')'
		{
			$$ = $2;
		}
	|		IF { theGrammar->out_opcode( AI_IF_THEN_ELSE, "if (then-else)" ); }
			condition
			consequent
			alternative
			{ theGrammar->end_opcode(); }
	|		BEGINP
		{
			theGrammar->out_opcode( AI_BEGINP );
//			Backpatch bp( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
//			theGrammar->_tblBackpatch.push_back( bp );
			theGrammar->_tblBackpatch.push_back( Backpatch( theGrammar->function() ) );
			theGrammar->out_number( Scalar::zero );
			theGrammar->end_opcode();
		}
			expr_list
		{
			Backpatch bp = theGrammar->_tblBackpatch.back();
			bp.doBackpatch( theGrammar->function() );
//			bp.backpatch( theGrammar->nProgramLines, theGrammar->pLineBuffer - theGrammar->lineBuffer );
			theGrammar->_tblBackpatch.pop_back();
		}
	|		DEFINE MAILBOX ACTOR NUMBER
		{
			theGrammar->add_mailbox( $3, Scalar( $4 ) );
		}
	|       	ACTOR '.' MAILBOX_TYPE
		{
			//theGrammar->out_opcode( AI_READ_MAILBOX, "read-mailbox" );
			//theGrammar->out_actor( $1 );
			//theGrammar->out_actor( $3 );
			//theGrammar->end_opcode();
		}
	|		MAILBOX_TYPE
		{
			//theGrammar->out_opcode( AI_READ_MAILBOX, "read-mailbox" );
			//theGrammar->out_actor( "self" );
			//theGrammar->out_actor( $1 );
			//theGrammar->end_opcode();
		}
	|		SET ACTOR '.' MAILBOX_TYPE
		{
			//theGrammar->out_opcode( AI_WRITE_TO_MAILBOX, "write-mailbox" );
			//theGrammar->out_actor( $2 );
			//theGrammar->out_actor( $4 );
			//theGrammar->end_opcode();
		}
			expr	//NUMBER
	|		SET MAILBOX_TYPE
		{
			//theGrammar->out_opcode( AI_WRITE_TO_MAILBOX, "write-mailbox" );
			//theGrammar->out_actor( "self" );
			//theGrammar->out_actor( $2 );
			//theGrammar->end_opcode();
		}
			expr	//NUMBER
	|		SEND_MESSAGE    { theGrammar->out_opcode( AI_SEND_MESSAGE   , "send-message" ); } expr expr expr { theGrammar->end_opcode(); }
	|		INC             { theGrammar->out_opcode( AI_INC            , "++" ); } expr { theGrammar->end_opcode(); }
	|		DEC             { theGrammar->out_opcode( AI_DEC            , "--" ); } expr { theGrammar->end_opcode(); }
	|		LSHIFT          { theGrammar->out_opcode( AI_LSHIFT         , "<<" ); } expr expr { theGrammar->end_opcode(); }
	|		RSHIFT          { theGrammar->out_opcode( AI_RSHIFT         , ">>" ); } expr expr { theGrammar->end_opcode(); }
	|		BAND            { theGrammar->out_opcode( AI_BAND           , "&" ); } expr expr { theGrammar->end_opcode(); }
	|		BOR             { theGrammar->out_opcode( AI_BOR            , "|" ); } expr expr { theGrammar->end_opcode(); }
	|		BNOT            { theGrammar->out_opcode( AI_BNOT           , "~" ); } expr { theGrammar->end_opcode(); }
	|		XOR             { theGrammar->out_opcode( AI_XOR            , "^" ); } expr expr { theGrammar->end_opcode(); }
	|		RANDOM          { theGrammar->out_opcode( AI_RANDOM         , "random" ); } expr { theGrammar->end_opcode(); }
	|		ROUND           { theGrammar->out_opcode( AI_ROUND          , "round" ); } expr { theGrammar->end_opcode(); }
	|		TRUNCATE        { theGrammar->out_opcode( AI_TRUNCATE       , "truncate" ); } expr { theGrammar->end_opcode(); }
	|		FLOOR           { theGrammar->out_opcode( AI_FLOOR          , "floor" ); } expr { theGrammar->end_opcode(); }
	|		CEILING	        { theGrammar->out_opcode( AI_CEILING	       , "ceiling" ); } expr { theGrammar->end_opcode(); }
	|		OP_ABS          { theGrammar->out_opcode( AI_OP_ABS         , "abs" ); } expr { theGrammar->end_opcode(); }
	|		ZERO_Q          { theGrammar->out_opcode( AI_ZERO_Q         , "zero?" ); } expr { theGrammar->end_opcode(); }
	|		POSITIVE_Q      { theGrammar->out_opcode( AI_POSITIVE_Q     , "positive?" ); } expr { theGrammar->end_opcode(); }
	|		NEGATIVE_Q      { theGrammar->out_opcode( AI_NEGATIVE_Q     , "negative?" ); } expr { theGrammar->end_opcode(); }
	|		OP_MIN          { theGrammar->out_opcode( AI_OP_MIN         , "min" ); } expr expr { theGrammar->end_opcode(); }
	|		OP_MAX          { theGrammar->out_opcode( AI_OP_MAX         , "max" ); } expr expr { theGrammar->end_opcode(); }
	|		SIN             { theGrammar->out_opcode( AI_SIN            , "sin" ); } expr { theGrammar->end_opcode(); }
	|		COS             { theGrammar->out_opcode( AI_COS            , "cos" ); } expr { theGrammar->end_opcode(); }
	|		TAN             { theGrammar->out_opcode( AI_TAN            , "tan" ); } expr { theGrammar->end_opcode(); }
	|		ASIN            { theGrammar->out_opcode( AI_ASIN           , "asin" ); } expr { theGrammar->end_opcode(); }
	|		ACOS            { theGrammar->out_opcode( AI_ACOS           , "acos" ); } expr { theGrammar->end_opcode(); }
	|		ATAN2           { theGrammar->out_opcode( AI_ATAN2          , "atan2" ); } expr expr { theGrammar->end_opcode(); }
	|		ATAN2QUICK      { theGrammar->out_opcode( AI_ATAN2QUICK     , "atan2quick" ); } expr expr { theGrammar->end_opcode(); }
 	|		WRITE           { theGrammar->out_opcode( AI_WRITE          , "write" ); } expr { theGrammar->end_opcode(); }
	|		WRITE           { theGrammar->out_opcode( AI_WRITE          , "write" ); } string { theGrammar->end_opcode(); }
 	|		WRITELN         { theGrammar->out_opcode( AI_WRITELN        , "writeln" ); } expr { theGrammar->end_opcode(); }
	|		WRITELN         { theGrammar->out_opcode( AI_WRITELN        , "writeln" ); } string { theGrammar->end_opcode(); }
	|		NEWLINE	        { theGrammar->out_opcode( AI_NEWLINE	       , "newline" ); theGrammar->end_opcode(); }
	|		CREATE_OBJECT   { theGrammar->out_opcode( AI_CREATE_OBJECT  , "create-object" ); } expr expr expr expr { theGrammar->end_opcode(); }
	|		FIND_CLASS      { theGrammar->out_opcode( AI_FIND_CLASS     , "find-class" ); } expr { theGrammar->end_opcode(); }
	|		SLEEP           { theGrammar->out_opcode( AI_SLEEP          , "sleep" ); } expr { theGrammar->end_opcode(); }
	|		EULER_TO_VECTOR { theGrammar->out_opcode( AI_EULER_TO_VECTOR, "spherical-to-cartesian" ); }
				expr expr expr expr expr expr
				{ theGrammar->end_opcode(); }
	|		VECTOR_TO_EULER { theGrammar->out_opcode( AI_VECTOR_TO_EULER, "cartesian-to-spherical" ); }
				expr expr expr expr expr
				 { theGrammar->end_opcode(); }
	|		EXIT { theGrammar->out_opcode( AI_EXIT, "exit" ); theGrammar->end_opcode(); }
	|		CHAR_LITERAL
		{
			theGrammar->out_opcode( AI_CHAR_LITERAL );
			theGrammar->out_int32( yylval.data.l );
			ID id( yylval.data.l );
			theGrammar->parseLog() << id;
			theGrammar->end_opcode();
		}
	|		ACTOR '.' ACTOR
		{
			theGrammar->out_opcode( AI_MEMBER, "(member)" );
			theGrammar->out_actor( $1 );
			theGrammar->out_actor( $3 );
			theGrammar->end_opcode();
		}
	;


string :
			STRING							{ theGrammar->out_string( $1 ); }
	;


%%

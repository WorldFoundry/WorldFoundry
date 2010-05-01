//==============================================================================
// aidump.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <iostream>

#include <math/scalar.hp>
#include <math/angle.hp>

#include "symbol.hp"
#include "obj/tokens_tab.h"
#include "airun.hp"
#include <version/version.hp>

long ffilesize( FILE *fp );


#if !defined( __VELOCITY__ )

//#define until( __condition__ )	while ( !( __condition__ ) )
int32 read_mailbox( int32 );

#endif

static bool bVerbose = true;

static int32* pScriptEnd;
static int32* pScript;
static char* pStringTable;

static int nIndentLevel = 0;

inline int32
pop()
{
	assert( pScript <= pScriptEnd );
	//assert( (void*)pScript < (void*)pStringTable );
	return *pScript++;
}


#define DISASSEMBLE( __opcode__, __nParameters__ ) \
{ \
	std::cout << "( " << __opcode__ << ' '; \
	for ( int i=0; i<__nParameters__; ++i ) \
		eval_expression(); \
	std::cout << ") "; \
	return Scalar::zero; \
}

Scalar
eval_expression()
{
	int32 expr = pop();

	if ( (expr & OPCODE_NAME) == OPCODE_NAME )
	{
		expr = ((unsigned)expr) >> 16;
//		int32 nTokens = pop();
		bVerbose && printf( "opcode: [%lx]\n", expr );
		switch ( expr )
		{
			case EXIT:
				DISASSEMBLE( "exit", 0 );

			case ROUND:
				DISASSEMBLE( "round", 1 );

			case TRUNCATE:
				DISASSEMBLE( "truncate", 1 );

			case FLOOR:
				DISASSEMBLE( "floor", 1 );

			case CEILING:
				DISASSEMBLE( "ceiling", 1 );

			case SIN:
				DISASSEMBLE( "sin", 1 );

			case COS:
				DISASSEMBLE( "cos", 1 );

			case ASIN:
				DISASSEMBLE( "asin", 1 );

			case ACOS:
				DISASSEMBLE( "acos", 1 );

			case ATAN2:
				DISASSEMBLE( "atan2", 2 );

			case ATAN2QUICK:
				DISASSEMBLE( "atan2quick", 2 );

			case OP_ABS:
				DISASSEMBLE( "abs", 1 );

			case OP_MIN:
				DISASSEMBLE( "min", 2 );

			case OP_MAX:
				DISASSEMBLE( "max", 2 );

			case ZERO_Q:
				DISASSEMBLE( "zero?", 1 );

			case POSITIVE_Q:
				DISASSEMBLE( "positive?", 1 );

			case NEGATIVE_Q:
				DISASSEMBLE( "negative?", 1 );

			case PLUS:
				DISASSEMBLE( "+", 2 );

			case MINUS:
				DISASSEMBLE( "-", 2 );

			case MULT:
				DISASSEMBLE( "*", 2 );

			case DIVIDE:
				DISASSEMBLE( "/", 2 );

			case QUOTIENT:
				DISASSEMBLE( "quotient", 2 );

			case REMAINDER:
				DISASSEMBLE( "remainder", 2 );

			case INC:
				DISASSEMBLE( "inc", 1 );

			case DEC:
				DISASSEMBLE( "dec", 1 );

			case LSHIFT:
				DISASSEMBLE( "<<", 2 );

			case RSHIFT:
				DISASSEMBLE( ">>", 2 );

			case LT:
				DISASSEMBLE( "<", 2 );

			case LTE:
				DISASSEMBLE( "<=", 2 );

			case GT:
				DISASSEMBLE( ">", 2 );

			case GTE:
				DISASSEMBLE( ">=", 2 );

			case EQ:
				DISASSEMBLE( "=", 2 );

			case NE:
				DISASSEMBLE( "!=", 2 );

			case SLEEP:
				DISASSEMBLE( "sleep", 1 );

			case READ_MAILBOX:
				DISASSEMBLE( "read-mailbox", 2 );

			case WRITE_TO_MAILBOX:
				DISASSEMBLE( "write-to-mailbox", 3 );

			case SEND_MESSAGE:
				DISASSEMBLE( "send-message", 3 );

			case IF:
			{
				std::cout << "( if ";
				eval_expression();
				eval_expression();
				std::cout << std::endl;
				++nIndentLevel;
				int i;
				for ( i=0; i<nIndentLevel; ++i )
					std::cout << '\t';
				eval_expression();  eval_expression();  std::cout << std::endl;
				for ( i=0; i<nIndentLevel; ++i )
					std::cout << '\t';
				eval_expression();
				std::cout << ") ";
				--nIndentLevel;
				return Scalar::zero;
			}

            case BAND:
				DISASSEMBLE( "&", 2 );

            case BOR:
				DISASSEMBLE( "|", 2 );

            case BNOT:
				DISASSEMBLE( "~", 1 );

            case XOR:
				DISASSEMBLE( "^", 2 );

			case LAND:
				DISASSEMBLE( "and", 2 );

			case LOR:
				DISASSEMBLE( "or", 2 );

			case LNOT:
				DISASSEMBLE( "not", 1 );

			case RANDOM:
				DISASSEMBLE( "random", 1 );

			case CREATE_OBJECT:
				DISASSEMBLE( "create-object", 4 );

			case FIND_CLASS:
				DISASSEMBLE( "find-class", 1 );

			case BRANCH:
				DISASSEMBLE( "BRANCH", 1 );

			case NEWLINE:
				DISASSEMBLE( "newline", 0 );

			case WRITE:
			{
				std::cout << "( write ";

				if ( (*pScript & STRING_NAME) == STRING_NAME )
				{
					int32 write = pop() >> 2;
					std::cout << '"' << pStringTable + write << '"' << ' ';
				}
				else
				{
					eval_expression();
				}

				std::cout << ") ";

				return Scalar::zero;
			}

			case WRITELN:
			{
				std::cout << "( writeln ";

				if ( (*pScript & STRING_NAME) == STRING_NAME )
				{
					int32 write = pop() >> 2;
					std::cout << '"' << pStringTable + write << '"' << ' ';
				}
				else
				{
					eval_expression();
				}

				std::cout << ") ";

				return Scalar::zero;
			}


			case EULER_TO_VECTOR:
				DISASSEMBLE( "euler-to-vector", 6 );

			case VECTOR_TO_EULER:
				DISASSEMBLE( "vector-to-euler", 5 );

			case SELF:
				DISASSEMBLE( "self", 0 );

			case BEGINP:
			{
				std::cout << "( begin ";
				int nCommands = eval_expression().WholePart();

				for ( int i=0; i<nCommands; ++i )
					eval_expression();
				std::cout << ')';
				return Scalar::zero;
			}

			default:
			{
				std::cerr << "Unknown opcode " << expr << std::endl;
				assert( 0 );
			}
		}
	}
	else if ( (expr & ACTOR_NAME) == ACTOR_NAME )
	{
		return Scalar(((unsigned)expr) >> 16);
	}
	else
	{
		assert( (expr & 3) == 0 );
//		printf( "K: [%d]\n", expr & 0xFFFF );
		std::cout << Scalar(expr) << ' ';
		return Scalar(expr);
	}

	printf( "Unknown parameter value on stack: [%lx]\n", expr );
	assert( 0 );
	return Scalar::zero;
}


Scalar
execute_program()
{
	Scalar val;

	assert( pScript );
	assert( pScriptEnd );
	assert( pScript <= pScriptEnd );

	while ( pScript < pScriptEnd )
	{
		val = eval_expression();
		std::cout << std::endl;
		//if ( bVerbose )
		//	std::cout << "statement = " << val << std::endl;
	}

	return val;
}

Scalar rc;

void
airun( int32* _pAiScript )
{
	pScript = _pAiScript + 1;
	pScriptEnd = _pAiScript + _pAiScript[ 0 ] + 1;
	pStringTable = (char*)pScriptEnd;
	// The script is required to have a guaranteed terminating condition
	rc = execute_program();
	//std::cout << "program evaluated to " << rc << std::endl;

//	return 0;
}



int
main( int argc, char* argv[] )
{
	if ( argc <= 1 )
	{
		std::cerr << "airun v" << szVersion << "  Copyright 1996-1997 World Foundry Group." << std::endl;
		std::cerr << "By William B. Norris IV" << std::endl;
		std::cerr << std::endl;
		std::cout << "Usage: airun <filename>.aib" << std::endl;
		return 10;
	}

	FILE* fp = fopen( argv[1], "rb" );
	assert( fp );

	int32 sizeOfFile = ffilesize( fp );
	assert( (sizeOfFile & 3) == 0 );
	int32* dataFile = (int32*)malloc( sizeOfFile );
	int nBytesRead = fread( dataFile, 1, sizeOfFile, fp );
	assert( nBytesRead == sizeOfFile );

	fclose( fp );

	airun( dataFile );

	return 0;
}

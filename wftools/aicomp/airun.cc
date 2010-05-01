//==============================================================================
// airun.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include "symbol.hp"
#include "obj/tokens_tab.h"
#include "airun.hp"
#include <version/version.hp>
#include <math/scalar.hp>

long ffilesize( FILE *fp );
void airun( int32* _pAiScript );

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





#if defined( DEBUG )
void
print_stack( int32* pScript, int nTokens )
{
	int32* _pScript = pScript;

	printf( "Stack: [%d]: ", nTokens );
	for ( int i=0; i<nTokens; ++i )
		printf( "%lx ", *_pScript-- );
}
#endif


Scalar
read_mailbox( int32 nMailbox )
{
	return Scalar( rand() % 500, 0 );
}


/*const*/ bool bVerbose = true;

static int32* pScript;
static int32* pScriptEnd;
static char* pStringTable;

inline int32
pop()
{
	assert( pScript <= pScriptEnd );
	//assert( (void*)pScript < (void*)pStringTable );
	return *pScript++;
}


Scalar
eval_expression()
{
	int32 expr = pop();

	if ( expr & OPCODE_NAME )
	{
		expr = ((unsigned)expr) >> 16;
		//int32 nTokens = pop();
		printf( "opcode: [%ld]\n", expr );
		switch ( expr )
		{
			case MEMBER:
			{
				int32 idxActor = eval_expression().WholePart();
				int32 cbOffset = eval_expression().WholePart();
				printf( "idxActor=%ld\tcbOffset=%ld\n", idxActor, cbOffset );
				assert( (cbOffset % 4) == 0 );
#if defined( __GAME__ )
				Actor* pActor = theLevel->getActor( idxActor );
				if ( pActor )
				{
					assert( ValidPtr( pActor ) );
					int32* ptr = (int32*)( ((char*)pActor->_oadData) + cbOffset );
					return Scalar( *ptr );
				}
#else
				return Scalar::zero;
#endif
			}

			case SELF:
#if defined( __GAME__ )
				assert( ValidPtr( _me ) );
				return QScalar( _me->GetActorIndex() );
#else
				return Scalar(1 << 16);
#endif

			case BEGINP:
			{
				Scalar ret;
				int32 nTokens = pop();
				int32* pEndP = pScript + nTokens;
				while ( pScript < pEndP )
					ret = eval_expression();
				return ret;
			}

			case CHAR_LITERAL:
			{
				unsigned long l = pop();
				Scalar s = Scalar( l );
				return s;
			}

			case EXIT:
				pScript = pScriptEnd;
				return Scalar::zero;

			case ROUND:
				return eval_expression();			// .Round()

			case TRUNCATE:
				return eval_expression();			// .Truncate()

			case FLOOR:
				return eval_expression();			// .Floor()

			case CEILING:
				return eval_expression();			// .Ceiling

			case OP_ABS:
			{
				Scalar e = eval_expression();
				return e >= Scalar::zero ? e : -e;
			}

			case OP_MIN:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 < n1 ? n2 : n1;
			}

			case OP_MAX:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 > n1 ? n2 : n1;
			}

			case ZERO_Q:
				return Scalar(( eval_expression() == Scalar::zero ) & 1);

			case POSITIVE_Q:
				return ( eval_expression() > Scalar::zero ) ? Scalar::one : Scalar::zero;

			case NEGATIVE_Q:
				return ( eval_expression() < Scalar::zero ) ? Scalar::one : Scalar::zero;

			case PLUS:
				return eval_expression() + eval_expression();

			case MINUS:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 - n1;
			}

			case MULT:
				return eval_expression() * eval_expression();

			case DIVIDE:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 / n1;
			}

			case QUOTIENT:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 / n1;
			}

			case REMAINDER:
			{
				int32 n1 = eval_expression().WholePart();
				int32 n2 = eval_expression().WholePart();
				return Scalar(n2 % n1);
			}

			case INC:
				return eval_expression() + Scalar::one;

			case DEC:
				return eval_expression() - Scalar::one;

			case LSHIFT:
			{
				int32 n1 = eval_expression().WholePart();
				int32 n2 = eval_expression().WholePart();
				return Scalar(n2 >= 0 ? n2 << n1 : n2 >> -n1);
			}

			case RSHIFT:
			{
				int32 n1 = eval_expression().WholePart();
				int32 n2 = eval_expression().WholePart();
				return Scalar(n2 >= 0 ? n2 >> n1 : n2 << -n1);
			}

			case LT:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 < n1 ? Scalar::one : Scalar::zero;
			}

			case LTE:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 <= n1 ? Scalar::one : Scalar::zero;
			}

			case GT:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 > n1 ? Scalar::one : Scalar::zero;
			}

			case GTE:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 >= n1 ? Scalar::one : Scalar::zero;
			}

			case EQ:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 == n1 ? Scalar::one : Scalar::zero;
			}

			case NE:
			{
				Scalar n1 = eval_expression();
				Scalar n2 = eval_expression();
				return n2 != n1 ? Scalar::one : Scalar::zero;
			}

			case READ_MAILBOX:
			{
				Scalar valMailbox;

				int32 idxActor = eval_expression().WholePart();
//				AssertMsg(idxActor > 0, *_me << ": Script tried to read mailbox from an invalid actor");
				int32 mailbox = eval_expression().WholePart();

#if defined( __GAME__ )
				bVerbose && printf( "Read actor %d's mailbox %d\n", idxActor, mailbox );
				Actor* pActor = theLevel->getActor( idxActor );
				AssertMsg( ValidPtr( pActor ) , *pActor << " doesn't refer to a currently active actor in " << *_me );
				valMailbox = pActor->getMailbox( mailbox );
#if DEBUG
				if ( !pActor && theLevel->_templateObjects[ idxActor ] )
					AssertMsg( 0, "tried to read to a templated actor's mailbox" );
#endif
#else
				valMailbox = read_mailbox( mailbox );
				if ( bVerbose )
					std::cout << "Read actor " << idxActor << "'s mailbox " << mailbox << " = " << valMailbox << std::endl;
#endif
				return valMailbox;
			}

			case SET:
			{
				int32 idxActor = eval_expression().WholePart();
				assert( idxActor > 0 );
				int32 mailbox = eval_expression().WholePart();
				Scalar value = eval_expression();

//				bVerbose && printf( "Write to actor %d's mailbox %d = %f\n", idxActor, mailbox, value/65536.0 );
				return value;
			}

			case WRITE_TO_MAILBOX:
			{
				int32 idxActor = eval_expression().WholePart();
				assert( idxActor > 0 );
				int32 mailbox = eval_expression().WholePart();
				Scalar value = eval_expression();

#if defined( __GAME__ )
				bVerbose && printf( "Write to actor %d's mailbox %d = %d\n", idxActor, mailbox, value );
				Actor* pActor = theLevel->getActor( idxActor );
				assert( ValidPtr( pActor ) );
#if DEBUG
				AssertMsg( !( !pActor && theLevel->_templateObjects[ idxActor ] ), "tried to write to a templated actor's mailbox" );
#endif
				pActor->setMailbox( mailbox, value );
#else
//				bVerbose && printf( "Write to actor %d's mailbox %d = %f\n", idxActor, mailbox, value/65536.0 );
#endif
				return value;
			}

			case SEND_MESSAGE:
			{
				int32 messageData = eval_expression().AsLong();
				int32 messageType = eval_expression().AsLong();
				int32 actor = eval_expression().WholePart();

				assert( messageType <= 0xFFFF );
				printf( "Actor=%ld  messageType=%ld  messageData=%ld\n", actor, messageType, messageData );
				//theObject->sendMessage( messageType, messageData );

				return Scalar(messageData);
			}

			case IF:
			{
				Scalar test = eval_expression();

				if ( test != Scalar::zero )
					pScript += 2;				// skip branch

				return eval_expression();
			 }

#if 0
			case COND:
			{
				assert( 0 );

				bElse = false;

				int32* pScriptCond = pScript;

				Scalar expr = eval_expression();
				printf( "expr=%f\n", expr/65536.0 );

				int32 test;
				int32 consequent;

				do
				{
					test = eval_expression();
					if ( bElse )
					{
						consequent = test;
						break;
					}

					consequent = eval_expression();

					printf( "test=%f\n", test/65536.0 );
					printf( "consequent=%f\n", consequent/65536.0 );
				}
				until ( expr == test );

				pScript = pScriptCond - nTokens;
				return consequent;
			}

			case ELSE:
				bElse = true;
				return eval_expression();
#endif

            case BAND:
				return Scalar( eval_expression().AsLong() & eval_expression().AsLong() );

            case BOR:
				return Scalar( eval_expression().AsLong() | eval_expression().AsLong() );

            case BNOT:
				return Scalar( ~( eval_expression().AsLong() ) );

            case XOR:
				return Scalar( eval_expression().AsLong() ^ eval_expression().AsLong() );

			case LAND:
				return Scalar::zero;
				//return (eval_expression() && eval_expression());	// Ensure 0 or 1

			case LOR:
				return Scalar::zero;
				//return (eval_expression() || eval_expression());	// Ensure 0 or 1

			case LNOT:
				return Scalar::zero;
				//return !eval_expression() & 1;							// Ensure 0 or 1

			case RANDOM:
				return Scalar::zero;
				//return Scalar( rand() % 100, 0 );

			case BRANCH:
			{
				int32 distance = pop();
				pScript += distance;
				return Scalar::zero;
			}

			case NEWLINE:
				std::cout << std::endl;
				return Scalar::zero;

			case WRITE:
			{
				Scalar ret;

				if ( (*pScript & STRING_NAME) == STRING_NAME )
				{
					int32 write = int32( pop() ) >> 2;
					std::cout << ( pStringTable + write );
					ret = Scalar::negativeOne;
				}
				else
				{
					ret = eval_expression();
					std::cout << ret;
				}

				return ret;
			}

			case WRITELN:
			{
				Scalar ret;

				if ( (*pScript & STRING_NAME) == STRING_NAME )
				{
					int32 write = int32( pop() ) >> 2;
					std::cout << ( pStringTable + write );
					ret = Scalar::negativeOne;
				}
				else
				{
					ret = eval_expression();
					std::cout << ret;
				}
				std::cout << std::endl;

				return ret;
			}


#if 0
			case NEGATE:
				return -eval_expression();
#endif

			default:
				std::cerr << "Invalid opcode " << expr << std::endl;
				assert( 0 );
		}
	}
	else if ( expr & ACTOR_NAME )
	{
		return Scalar(((unsigned)expr) >> 16);
	}
	else
	{
		assert( (expr & 3) == 0 );
//		printf( "K: [%d]\n", expr & 0xFFFF );
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
//#if DEBUG > 0
		if ( bVerbose )
			std::cout << std::endl << "statement = " << val << std::endl;
//#endif
	}

	return val;
}

Scalar rc;

void
airun( int32* _pAiScript )
{
#if defined( __GAME__ )
	if ( theLevel->getWallClock() >= _sleepUntil )
#endif
	{
		pScript = _pAiScript + 1;
		pScriptEnd = _pAiScript + _pAiScript[ 0 ] + 1;
		pStringTable = (char*)pScriptEnd;

		rc = execute_program();
//#if DEBUG > 0
		if ( bVerbose )
			std::cout << "program evaluated to " << rc << std::endl;
//#endif
	}
//	return 0;
}

%{
//==============================================================================
// lang.y: Copyright (c) 1996-1999, World Foundry Group  
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
#include <stdlib.h>
#include <fstream>
#include <ctime>
using namespace std;
#include "langlex.hpp"
#include "grammar.hpp"
#include "lexyy.h"
#include <iffwrite/iffwrite.hp>

extern int yylex();

extern strFlexLexer* theLexer;
extern bool bVerbose;
unsigned long startPosOverride;
unsigned long lengthOverride;

static char szChunkIdentifier[ 256 ];

#include <iffwrite/fixed.hp>

%}

%union X {
	struct
	{
  		unsigned long val;
		int sizeOverride;
	} integer;
	struct
	{
		char* str;
		int sizeOverride;
	} string;
	struct real
	{
		double dval;
		struct size_specifier size_specifier;
		//int sizeOverride;
	} real;
	struct size_specifier size_specifier;
	unsigned long fourcc;
}


%token <integer> INTEGER
%token <string> STRING
%token <fourcc> CHAR_LITERAL
%token <real> REAL
%token TIMESTAMP ALIGN DOUBLE_COLON OFFSETOF SIZEOF FILLCHAR START LENGTH PRECISION
%token PLUS MINUS
%token <size_specifier> PRECISION_SPECIFIER
%type <integer.val> item expr


%%

statement_list :
		statement_list statement
	|	statement
	;


statement :
		chunk
	;


chunk :
		'{'
			CHAR_LITERAL		{ theGrammar->_iff->enterChunk( ID( $2 ) ); }
			chunk_statement_list
		'}' 				{ theGrammar->_iff->exitChunk(); }
	;


chunk_statement_list :
		chunk_statement_list chunk_statement
	|	chunk_statement
	;

chunk_statement :
		chunk
	|	alignment
	|	fillchar			{}
	|	expr				{}
	|	{}
	;

fillchar :
		FILLCHAR '(' INTEGER ')'	{ theGrammar->_iff->fillChar( $3.val ); }
	;

alignment :
		ALIGN '(' INTEGER ')'
		{
			assert( theGrammar );
			assert( theGrammar->_iff );
			if ( $3.val == 0 )
				theGrammar->Error( "Align to 0 doesn't make sense [ignoring]" );
			else
				theGrammar->_iff->alignFunction( $3.val );
		}
	;

expr_list :
		expr_list expr			{}
	|	item				{}
	;

size_specifier :
			'Y'				{ State newState = theGrammar->vecState.back();
							newState.sizeOverride( 1 );
							theGrammar->vecState.push_back( newState );
							}
	|		'W'				{ State newState = theGrammar->vecState.back();
							newState.sizeOverride( 2 );
							theGrammar->vecState.push_back( newState );
							}
	|		'L'				{ State newState = theGrammar->vecState.back();
							newState.sizeOverride( 4 );
							theGrammar->vecState.push_back( newState );
							}
	;

precision_specifier :
			PRECISION '(' PRECISION_SPECIFIER ')'	{
							State newState = theGrammar->vecState.back();
							newState.precision( $3 );
							theGrammar->vecState.push_back( newState );
							}
	;

chunkSpecifier :
		DOUBLE_COLON CHAR_LITERAL			{ ID id( $2 );
								strcpy( szChunkIdentifier, "::'" );
								strcat( szChunkIdentifier, id.name() );
								strcat( szChunkIdentifier, "'" );
								//cout << szChunkIdentifier << endl;
								}
	|	chunkSpecifier DOUBLE_COLON CHAR_LITERAL	{ ID id( $3 );
								strcat( szChunkIdentifier, "::'" );
								strcat( szChunkIdentifier, id.name() );
								strcat( szChunkIdentifier, "'" );
								//cout << szChunkIdentifier << endl;
								}
	;


extractSpecifierList :
		extractSpecifierList extractSpecifier		{}
	|	extractSpecifier				{}
	|
	;

extractSpecifier :
		START '(' INTEGER ')'				{ startPosOverride = $3.val; }
	|	LENGTH '(' INTEGER ')'				{ lengthOverride = $3.val; }
	;


state_push :
		'{' precision_specifier expr_list '}'
	|	'{' size_specifier expr_list '}'
	;

string_list :
		string_list STRING
		{
			//printf( "$1 = [%s]\n", $1 );
			//printf( "$2 = [%s]\n", $2 );
			theGrammar->_iff->out_string_continue( $2.str );
		}
	|	STRING
		{
			//printf( "single STRING = [%s]\n", $1 );
			assert( theGrammar );
			assert( theGrammar->_iff );
			theGrammar->_iff->out_string( $1.str );
			if ( $1.sizeOverride )
			{
			  int totalCharsRequired = strlen($1.str) + 1;
			  if ( totalCharsRequired > $1.sizeOverride )
			  {
			    int cbOverrun = totalCharsRequired - $1.sizeOverride;
			    theGrammar->Warning( "string \"%s\" longer than specified size of %d by %d character%s", $1.str, $1.sizeOverride, cbOverrun, cbOverrun > 1 ? "s" : "" );
			  }
			  else
			  {
			    for ( int i=0; i<$1.sizeOverride - totalCharsRequired; ++i )
			      theGrammar->_iff->out_int8( 0 );
			  }
			}
			free( $1.str );
		}
	;

item :
		state_push
		{
			theGrammar->vecState.pop_back();
		}
	|	REAL
		{
			assert( theGrammar );
			assert( theGrammar->_iff );

			*( theGrammar->_iff ) << Fixed( $1.size_specifier, $1.dval );
			//cout << Fixed( $1.real.size_specifier, $1.real.dval ) << endl;
		}
	|	INTEGER
		{
			assert( theGrammar );
			assert( theGrammar->_iff );

			int sizeOverride = $1.sizeOverride;
			//printf( "%d sizeOverride=%d", $1, sizeOverride );
			if ( !sizeOverride )
			{
				//printf( "Applying default size of %d\n", theGrammar->vecState.back() );
				sizeOverride = theGrammar->vecState.back().sizeOverride();
			}

			switch ( sizeOverride )
			{
				case 1:
				{
					//cout << "out_int8( " << $1.val << " )" << endl;
					theGrammar->_iff->out_int8( $1.val );
					if($1.val > 255)
						theGrammar->Error( "value doesn't fit into an int8" );	
					break;
				}

				case 2:
				{
					//cout << "out_int16( " << $1.val << " )" << endl;
					if($1.val > 65535)
						theGrammar->Error( "value doesn't fit into an int16" );	
					theGrammar->_iff->out_int16( $1.val );
					break;
				}

				case 4:
				{
					//cout << "out_int32( " << $1.val << " )" << endl;
					if($1.val > 0x7fffffff)
						theGrammar->Error( "value doesn't fit into an int32" );	
					theGrammar->_iff->out_int32( $1.val );
					break;
				}

				default:
					cerr << "sizeOverride = " << sizeOverride << endl;
					assert( 0 );
			}
		}
	|	string_list	{}
	|	'[' { startPosOverride = 0; lengthOverride = ~0; } STRING extractSpecifierList ']'
		{
			assert( theGrammar );
			assert( theGrammar->_iff );
			File file( $3.str, startPosOverride, lengthOverride );
			*(theGrammar->_iff) << file;
		}
	|	CHAR_LITERAL
		{
			assert( theGrammar );
			assert( theGrammar->_iff );
			*theGrammar->_iff << ID( $1 );
		}
	|	TIMESTAMP
		{
			assert( theGrammar );
			assert( theGrammar->_iff );
			Timestamp ts;
			*(theGrammar->_iff) << ts;
		}
	|	OFFSETOF '(' chunkSpecifier ',' INTEGER ')'
		{
			assert( theGrammar );
			assert( theGrammar->_iff );
			const ChunkSizeBackpatch* cs = theGrammar->_iff->findSymbol( szChunkIdentifier );
			if ( cs )
			{
				theGrammar->_iff->out_int32( cs->GetPos() + $5.val );
				if ( bVerbose ) cout << "offsetof( " << szChunkIdentifier << " ) = " << cs->GetPos() << ' ';
			}
			else
			{
				Backpatch* bp = new Backpatch( Backpatch::TYPE_OFFSETOF, $5.val );
				assert( bp );
				bp->pos = theGrammar->binout->tellp();
				bp->szChunkIdentifier = strdup( szChunkIdentifier );
				theGrammar->_backpatchSizeOffset.push_back( bp );
				theGrammar->_iff->out_int32( ~0 );
			}
		}
	|	OFFSETOF '(' chunkSpecifier ')'
		{

			assert( theGrammar );
			assert( theGrammar->_iff );
			const ChunkSizeBackpatch* cs = theGrammar->_iff->findSymbol( szChunkIdentifier );
			if ( cs )
			{
				theGrammar->_iff->out_int32( cs->GetPos() );
				if ( bVerbose ) cout << "offsetof( " << szChunkIdentifier << " ) = " << cs->GetPos() << ' ';
			}
			else
			{
				Backpatch* bp = new Backpatch( Backpatch::TYPE_OFFSETOF );
				assert( bp );
				bp->pos = theGrammar->binout->tellp();
				bp->szChunkIdentifier = strdup( szChunkIdentifier );
				theGrammar->_backpatchSizeOffset.push_back( bp );
				theGrammar->_iff->out_int32( ~0 );
			}
		}
	|	SIZEOF '(' chunkSpecifier ')'
		{
			assert( theGrammar );
			assert( theGrammar->_iff );
			const ChunkSizeBackpatch* cs = theGrammar->_iff->findSymbol( szChunkIdentifier );
			if ( cs )
			{
				theGrammar->_iff->out_int32( cs->GetSize() );
				if ( bVerbose ) cout << "sizeof( " << szChunkIdentifier << " ) = " << cs->GetSize() << ' ';
			}
			else
			{
				Backpatch* bp = new Backpatch( Backpatch::TYPE_SIZEOF );
				assert( bp );
				bp->pos = theGrammar->binout->tellp();
				bp->szChunkIdentifier = strdup( szChunkIdentifier );
				theGrammar->_backpatchSizeOffset.push_back( bp );
				theGrammar->_iff->out_int32( 0 );
			}
		}
	;


expr :
		item			{ $$ = $1; }
	|	expr PLUS expr          { $$ = $1 + $3; printf( "%ld + %ld = %ld\n", $1, $3, $$ ); }
	|	expr MINUS expr
	;

%%

int
yylex()
{
	assert( theLexer );
	return theLexer->yylex();
}

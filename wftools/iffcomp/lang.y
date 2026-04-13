//==============================================================================
// lang.y: Copyright (c) 1996-1999, 2026, World Foundry Group
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

%require "3.8"
%language "c++"
%define api.parser.class {LangParser}
%define api.namespace {yy}
%define api.value.type variant
%define api.token.constructor
%define parse.error detailed
%define parse.trace
%locations

%code requires {
  #include <string>
  #include <iffwrite/fixed.hp>
  class Grammar;
  class LangLexer;
  struct IntLit  { unsigned long val; int sizeOverride; };
  struct StrLit  { std::string str; int sizeOverride; };
  struct RealLit { double dval; size_specifier precision; };
}

%param { Grammar& g }
%param { LangLexer& lex }

%code {
  #include <cassert>
  #include <cstring>
  #include <cstdlib>
  #include <cstdio>
  #include <fstream>
  #include <iostream>
  #include <ctime>
  #include "grammar.hpp"
  #include "langlex.hpp"
  #include <iffwrite/iffwrite.hp>
  #include <iffwrite/fixed.hp>

  extern bool bVerbose;

  static unsigned long startPosOverride;
  static unsigned long lengthOverride;
  static char szChunkIdentifier[256];

  // defined by flex (generated from lang.l via YY_DECL)
  yy::LangParser::symbol_type yylex(Grammar& g, LangLexer& lex);
}

%token <IntLit>         INTEGER
%token <StrLit>         STRING
%token <unsigned long>  CHAR_LITERAL
%token <RealLit>        REAL
%token <size_specifier> PRECISION_SPECIFIER

%token TIMESTAMP ALIGN DOUBLE_COLON OFFSETOF SIZEOF FILLCHAR START LENGTH PRECISION
%token PLUS MINUS

%token LBRACE  "{"
%token RBRACE  "}"
%token LPAREN  "("
%token RPAREN  ")"
%token LBRACK  "["
%token RBRACK  "]"
%token T_COMMA ","
%token SIZE_Y  "Y"
%token SIZE_W  "W"
%token SIZE_L  "L"

%type <unsigned long> item expr

%%

statement_list
    : statement_list statement
    | statement
    ;


statement
    : chunk
    ;


chunk
    : LBRACE CHAR_LITERAL                       { g._iff->enterChunk( ID( $2 ) ); }
      chunk_statement_list
      RBRACE                                    { g._iff->exitChunk(); }
    ;


chunk_statement_list
    : chunk_statement_list chunk_statement
    | chunk_statement
    ;

chunk_statement
    : chunk
    | alignment
    | fillchar                                  {}
    | expr                                      {}
    | %empty                                    {}
    ;

fillchar
    : FILLCHAR LPAREN INTEGER RPAREN            { g._iff->fillChar( $3.val ); }
    ;

alignment
    : ALIGN LPAREN INTEGER RPAREN
        {
            assert( g._iff );
            if ( $3.val == 0 )
                g.Error( "Align to 0 doesn't make sense [ignoring]" );
            else
                g._iff->alignFunction( $3.val );
        }
    ;

expr_list
    : expr_list expr                            {}
    | item                                      {}
    ;

size_specifier
    : SIZE_Y    { State newState = g.vecState.back();
                  newState.sizeOverride( 1 );
                  g.vecState.push_back( newState );
                }
    | SIZE_W    { State newState = g.vecState.back();
                  newState.sizeOverride( 2 );
                  g.vecState.push_back( newState );
                }
    | SIZE_L    { State newState = g.vecState.back();
                  newState.sizeOverride( 4 );
                  g.vecState.push_back( newState );
                }
    ;

precision_specifier
    : PRECISION LPAREN PRECISION_SPECIFIER RPAREN
        {
            State newState = g.vecState.back();
            newState.precision( $3 );
            g.vecState.push_back( newState );
        }
    ;

chunkSpecifier
    : DOUBLE_COLON CHAR_LITERAL
        {
            ID id( $2 );
            std::strcpy( szChunkIdentifier, "::'" );
            std::strcat( szChunkIdentifier, id.name() );
            std::strcat( szChunkIdentifier, "'" );
        }
    | chunkSpecifier DOUBLE_COLON CHAR_LITERAL
        {
            ID id( $3 );
            std::strcat( szChunkIdentifier, "::'" );
            std::strcat( szChunkIdentifier, id.name() );
            std::strcat( szChunkIdentifier, "'" );
        }
    ;


extractSpecifierList
    : extractSpecifierList extractSpecifier     {}
    | extractSpecifier                          {}
    | %empty                                    {}
    ;

extractSpecifier
    : START LPAREN INTEGER RPAREN               { startPosOverride = $3.val; }
    | LENGTH LPAREN INTEGER RPAREN              { lengthOverride   = $3.val; }
    ;


state_push
    : LBRACE precision_specifier expr_list RBRACE
    | LBRACE size_specifier      expr_list RBRACE
    ;

string_list
    : string_list STRING
        {
            g._iff->out_string_continue( $2.str.c_str() );
        }
    | STRING
        {
            assert( g._iff );
            g._iff->out_string( $1.str.c_str() );
            if ( $1.sizeOverride )
            {
                int totalCharsRequired = static_cast<int>( $1.str.size() ) + 1;
                if ( totalCharsRequired > $1.sizeOverride )
                {
                    int cbOverrun = totalCharsRequired - $1.sizeOverride;
                    g.Warning( "string \"%s\" longer than specified size of %d by %d character%s",
                               $1.str.c_str(), $1.sizeOverride, cbOverrun, cbOverrun > 1 ? "s" : "" );
                }
                else
                {
                    for ( int i = 0; i < $1.sizeOverride - totalCharsRequired; ++i )
                        g._iff->out_int8( 0 );
                }
            }
        }
    ;

item
    : state_push
        {
            g.vecState.pop_back();
            $$ = 0;
        }
    | REAL
        {
            assert( g._iff );
            *( g._iff ) << Fixed( $1.precision, $1.dval );
            $$ = 0;
        }
    | INTEGER
        {
            assert( g._iff );

            int sizeOverride = $1.sizeOverride;
            if ( !sizeOverride )
                sizeOverride = g.vecState.back().sizeOverride();

            switch ( sizeOverride )
            {
                case 1:
                    g._iff->out_int8( $1.val );
                    if ( $1.val > 255 )
                        g.Error( "value doesn't fit into an int8" );
                    break;

                case 2:
                    if ( $1.val > 65535 )
                        g.Error( "value doesn't fit into an int16" );
                    g._iff->out_int16( $1.val );
                    break;

                case 4:
                    if ( $1.val > 0x7fffffffu )
                        g.Error( "value doesn't fit into an int32" );
                    g._iff->out_int32( $1.val );
                    break;

                default:
                    std::cerr << "sizeOverride = " << sizeOverride << std::endl;
                    assert( 0 );
            }
            $$ = $1.val;
        }
    | string_list                               { $$ = 0; }
    | LBRACK { startPosOverride = 0; lengthOverride = ~0ul; }
      STRING extractSpecifierList RBRACK
        {
            assert( g._iff );
            File file( $3.str.c_str(), startPosOverride, lengthOverride );
            *( g._iff ) << file;
            $$ = 0;
        }
    | CHAR_LITERAL
        {
            assert( g._iff );
            *g._iff << ID( $1 );
            $$ = 0;
        }
    | TIMESTAMP
        {
            assert( g._iff );
            Timestamp ts;
            *( g._iff ) << ts;
            $$ = 0;
        }
    | OFFSETOF LPAREN chunkSpecifier T_COMMA INTEGER RPAREN
        {
            assert( g._iff );
            const ChunkSizeBackpatch* cs = g._iff->findSymbol( szChunkIdentifier );
            if ( cs )
            {
                g._iff->out_int32( cs->GetPos() + $5.val );
                if ( bVerbose ) std::cout << "offsetof( " << szChunkIdentifier << " ) = " << cs->GetPos() << ' ';
            }
            else
            {
                Backpatch* bp = new Backpatch( Backpatch::TYPE_OFFSETOF, $5.val );
                bp->pos = g.binout->tellp();
                bp->szChunkIdentifier = szChunkIdentifier;
                g._backpatchSizeOffset.push_back( bp );
                g._iff->out_int32( ~0u );
            }
            $$ = 0;
        }
    | OFFSETOF LPAREN chunkSpecifier RPAREN
        {
            assert( g._iff );
            const ChunkSizeBackpatch* cs = g._iff->findSymbol( szChunkIdentifier );
            if ( cs )
            {
                g._iff->out_int32( cs->GetPos() );
                if ( bVerbose ) std::cout << "offsetof( " << szChunkIdentifier << " ) = " << cs->GetPos() << ' ';
            }
            else
            {
                Backpatch* bp = new Backpatch( Backpatch::TYPE_OFFSETOF );
                bp->pos = g.binout->tellp();
                bp->szChunkIdentifier = szChunkIdentifier;
                g._backpatchSizeOffset.push_back( bp );
                g._iff->out_int32( ~0u );
            }
            $$ = 0;
        }
    | SIZEOF LPAREN chunkSpecifier RPAREN
        {
            assert( g._iff );
            const ChunkSizeBackpatch* cs = g._iff->findSymbol( szChunkIdentifier );
            if ( cs )
            {
                g._iff->out_int32( cs->GetSize() );
                if ( bVerbose ) std::cout << "sizeof( " << szChunkIdentifier << " ) = " << cs->GetSize() << ' ';
            }
            else
            {
                Backpatch* bp = new Backpatch( Backpatch::TYPE_SIZEOF );
                bp->pos = g.binout->tellp();
                bp->szChunkIdentifier = szChunkIdentifier;
                g._backpatchSizeOffset.push_back( bp );
                g._iff->out_int32( 0 );
            }
            $$ = 0;
        }
    ;


expr
    : item                   { $$ = $1; }
    | expr PLUS expr         { $$ = $1 + $3; }
    | expr MINUS expr        { $$ = $1 - $3; }
    ;

%%

void yy::LangParser::error( const location_type& /*loc*/, const std::string& msg )
{
    g.Error( "%s", msg.c_str() );
}

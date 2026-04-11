%{
//==============================================================================
// lang.l: Copyright (c) 1996-1999, 2026, World Foundry Group
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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <iffwrite/fixed.hp>
#include "grammar.hpp"
#include "langlex.hpp"
#include "lang.tab.hh"

// Tell flex that yylex returns a bison symbol_type and takes our parser params.
#define YY_DECL  yy::LangParser::symbol_type yylex( Grammar& g, LangLexer& lex )

// Update the location column on every token match.
#define YY_USER_ACTION  lex.loc().columns( yyleng );

static int
checkSize( const char* str )
{
    int sizeOverride = 0;
    char ch = *( str + std::strlen( str ) - 1 );
    if ( ch == 'y' )      sizeOverride = 1;
    else if ( ch == 'w' ) sizeOverride = 2;
    else if ( ch == 'l' ) sizeOverride = 4;
    return sizeOverride;
}

static size_specifier
parse_size_specifier( const char* szText, Grammar& g )
{
    size_specifier ret = { 1, 15, 16 };
    std::sscanf( szText, "%d.%d.%d", &ret.sign, &ret.whole, &ret.fraction );
    if ( ( ret.sign < 0 ) || ( ret.sign > 1 ) )
    {
        g.Warning( "Sign must be either 0 or 1 [assuming 1]" );
        ret.sign = 1;
    }
    return ret;
}

%}

%option noyywrap
%option nounput
%option never-interactive

%x INCLUDE

%%

include[ \t]+\"                 { BEGIN(INCLUDE); }

<INCLUDE>[^\">]*                {
    int chClosingQuote = yyinput();
    if ( chClosingQuote == '>' )
        lex.push_system_include( yytext );
    else
        lex.push_include( yytext );
    BEGIN(INITIAL);
}

include[ \t]+\<                 { BEGIN(INCLUDE); }

\.timestamp                     { return yy::LangParser::make_TIMESTAMP( lex.loc() ); }
\.align                         { return yy::LangParser::make_ALIGN( lex.loc() ); }
\.offsetof                      { return yy::LangParser::make_OFFSETOF( lex.loc() ); }
\.sizeof                        { return yy::LangParser::make_SIZEOF( lex.loc() ); }
\.fillchar                      { return yy::LangParser::make_FILLCHAR( lex.loc() ); }
\.start                         { return yy::LangParser::make_START( lex.loc() ); }
\.length                        { return yy::LangParser::make_LENGTH( lex.loc() ); }
\.precision                     { return yy::LangParser::make_PRECISION( lex.loc() ); }

"::"                            { return yy::LangParser::make_DOUBLE_COLON( lex.loc() ); }
"+"                             { return yy::LangParser::make_PLUS( lex.loc() ); }
"-"                             { return yy::LangParser::make_MINUS( lex.loc() ); }

"{"                             { return yy::LangParser::make_LBRACE( lex.loc() ); }
"}"                             { return yy::LangParser::make_RBRACE( lex.loc() ); }
"("                             { return yy::LangParser::make_LPAREN( lex.loc() ); }
")"                             { return yy::LangParser::make_RPAREN( lex.loc() ); }
"["                             { return yy::LangParser::make_LBRACK( lex.loc() ); }
"]"                             { return yy::LangParser::make_RBRACK( lex.loc() ); }
","                             { return yy::LangParser::make_T_COMMA( lex.loc() ); }
"Y"                             { return yy::LangParser::make_SIZE_Y( lex.loc() ); }
"W"                             { return yy::LangParser::make_SIZE_W( lex.loc() ); }
"L"                             { return yy::LangParser::make_SIZE_L( lex.loc() ); }

'.{1,4}'    {
    int shift = 24;
    unsigned long v = 0;
    int lenID = static_cast<int>( std::strlen( yytext ) ) - 2;
    for ( int i = 0; i < 4; ++i )
    {
        unsigned long n = i < lenID ? static_cast<unsigned char>( yytext[ 1 + i ] ) : 0u;
        v |= n << shift;
        shift -= 8;
    }
    return yy::LangParser::make_CHAR_LITERAL( v, lex.loc() );
}

[0-9]+\.[0-9]+\.[0-9]+   {
    return yy::LangParser::make_PRECISION_SPECIFIER( parse_size_specifier( yytext, g ), lex.loc() );
}

\$[0-9A-Fa-f]+[ywl]?    {
    IntLit il{};
    std::sscanf( yytext + 1, "%lx", &il.val );
    il.sizeOverride = checkSize( yytext );
    return yy::LangParser::make_INTEGER( il, lex.loc() );
}


-?[0-9]+[ywl]?          {
    IntLit il{};
    long tmp = 0;
    std::sscanf( yytext, "%ld", &tmp );
    il.val = static_cast<unsigned long>( tmp );
    il.sizeOverride = checkSize( yytext );
    return yy::LangParser::make_INTEGER( il, lex.loc() );
}


-?(([0-9]*(\.[0-9]+)([eE][+-]?[0-9]+)?)|([0-9]+)){1}(\([0-9]+\.[0-9]+\.[0-9]+\))?    {
    RealLit r{};
    std::istringstream buf( std::string( yytext, yyleng ) );
    buf >> r.dval;
    const char* szPrecisionOverride = std::strchr( yytext, '(' );
    r.precision = szPrecisionOverride
        ? parse_size_specifier( szPrecisionOverride + 1, g )
        : g.vecState.back().precision();
    return yy::LangParser::make_REAL( r, lex.loc() );
}


\"(\\\"|\\\\|[^\"\n])*[\"\n](\([0-9]+\))?    {
    // Locate the closing '"' or '\n' that terminated the string proper;
    // anything after is an optional "(N)" size override.
    const char* p = yytext + 1;
    const char* endq = nullptr;
    while ( *p )
    {
        if ( p[0] == '\\' && p[1] )  { p += 2; continue; }
        if ( p[0] == '"' || p[0] == '\n' ) { endq = p; break; }
        ++p;
    }

    StrLit sl{};
    if ( endq )
    {
        sl.str.assign( yytext + 1, endq - ( yytext + 1 ) );
        if ( *endq == '\n' )
        {
            g.Warning( "Unterminated string (%s)\n", sl.str.c_str() );
            lex.loc().lines( 1 );
            lex.nextLine();
        }
        if ( *( endq + 1 ) == '(' )
            sl.sizeOverride = std::atoi( endq + 2 );
    }
    else
    {
        sl.str.assign( yytext + 1 );
    }
    return yy::LangParser::make_STRING( sl, lex.loc() );
}

"//".*                          { /* line comment */ }

\n                              {
    lex.loc().lines( 1 );
    lex.nextLine();
}

[\t ]+                          { /* whitespace */ }

<<EOF>>                         {
    if ( !lex.pop_include() )
        return yy::LangParser::make_YYEOF( lex.loc() );
}

.                               {
    g.Error( "unexpected character '%c' (0x%02x)", yytext[0], (unsigned char)yytext[0] );
}

%%

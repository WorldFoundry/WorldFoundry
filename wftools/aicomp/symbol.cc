//==============================================================================
// symbol.cc: Copyright (c) 1997-1999, World Foundry Group  
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
//#include <string>
//#include <vector>

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

int programReturnCode;

#include <version/version.hp>

extern bool bVerbose;	// = true;			// Verbose output
extern bool bQuiet;	// = false;			// No error output

aiFlexLexer* theLexer;

#include "grammar.hp"

void beep();
int yyparse();

void
aiGrammar::yyparse()
{
	::yyparse();
}

extern aiGrammar* theGrammar;


#if !defined( STL_SYMBOLS )
Symbol*
aiGrammar::find_symbol( const char* szSymbolName )
{
	for ( int i=0; i<nSymbols; ++i )
	{
		if ( strcmp( tblSymbols[i].szSymbolName, szSymbolName ) == 0 )
			return &tblSymbols[i];
	}

	return NULL;
}


void
aiGrammar::add_symbol( const char* szSymbolName, Scalar nSymbolValue )
{
	if ( find_symbol( szSymbolName ) )
		theGrammar->Error( "Duplicate symbol \"%s\"", szSymbolName );
	else
	{
		assert( nSymbols <= sizeof( tblSymbols ) / sizeof( tblSymbols[0] ) );
		tblSymbols[ nSymbols ].szSymbolName = (char*)szSymbolName;
		tblSymbols[ nSymbols ].nSymbolValue = nSymbolValue;
		++nSymbols;
	}
}


Symbol*
aiGrammar::find_mailbox( const char* szSymbolName )
{
	for ( int i=0; i<nMailboxes; ++i )
	{
		if ( strcmp( tblMailboxes[i].szSymbolName, szSymbolName ) == 0 )
			return &tblMailboxes[i];
	}

	return NULL;
}


void
aiGrammar::add_mailbox( const char* szMailboxName, Scalar nSymbolValue )
{
	if ( find_mailbox( szMailboxName ) )
		theGrammar->Error( "Duplicate mailbox \"%s\"", szMailboxName );
	else
	{
		assert( nMailboxes <= sizeof( tblMailboxes ) / sizeof( tblMailboxes[0] ) );
		tblMailboxes[ nMailboxes ].szSymbolName = (char*)szMailboxName;
		tblMailboxes[ nMailboxes ].nSymbolValue = nSymbolValue;
		++nMailboxes;
	}
}
#endif


extern "C" int
yylex()
{
	assert( theLexer );
	return theLexer->yylex();
}


/*extern "C" __declspec( dllexport )*/ int /*__stdcall*/
aicomp( const char* _szInputFile )
{
	assert( _szInputFile );
	assert( *_szInputFile );
	//szInputFile = _szInputFile;

	programReturnCode = 0;

//?STL	ifstream* inp = new ifstream( _szInputFile, ios::in | ios::nocreate );
	ifstream* inp = new ifstream( _szInputFile, ios::in );
	assert( inp );
	if ( !inp->good() )
	{
		cerr << "Unable to open input file \"" << _szInputFile << "\"" << endl;
		return 10;
	}

	theGrammar = new aiGrammar( _szInputFile, inp );
	assert( theGrammar );

	theGrammar->yyparse();

	delete theGrammar;

	return programReturnCode;
}

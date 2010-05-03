//==============================================================================
// langlex.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include <pigsys/assert.hp>
#include <stdio.h>
#include <string>
#include <fstream>
using namespace std;
#include "langlex.hpp"

// Don't like this...
#define YY_BUF_SIZE 16384
#include "grammar.hpp"

strFlexLexer::strFlexLexer( istream* arg_yyin, ostream* arg_yyout ) :
		yyFlexLexer( arg_yyin, arg_yyout )
{
}


int 
yyFlexLexer::yywrap()
{
  return 1;
}


int 
strFlexLexer::yywrap()
{
  return 1;
}


const char*
strFlexLexer::filename()
{
	return include()->szFilename;
}


const char*
strFlexLexer::currentLine()
{
	return include()->_szCurrentLine;
}


const char*
strFlexLexer::currentLine( const char* szCurrentLine )
{
	return include()->_szCurrentLine = (char*)szCurrentLine;
}


FileLineInfo*
strFlexLexer::include()
{
	if ( _fileLineInfo.size() == 0 )
	{
		FileLineInfo* fli = new FileLineInfo(
			1,
			"",
			theGrammar->filename(),
			yy_current_buffer );
		assert( fli );
		_fileLineInfo.push_back( fli );
	}
	return *( _fileLineInfo.end() - 1 );
}


void
strFlexLexer::push_system_include( const char* szIncludeFile )
{
	char szSystemIncludeFile[ PATH_MAX ];

	char* szVelocityDir = getenv( "WF_DIR" );
	assert( szVelocityDir );
	sprintf( szSystemIncludeFile, "%s\\%s", szVelocityDir, szIncludeFile );

	push_include( szSystemIncludeFile );
}


void
strFlexLexer::push_include( const char* szIncludeFile )
{
//	printf( "Opening file [%s]\n", szIncludeFile );

    // kts added 2/24/2001 21:10:46 so that top level yy_current_buffer gets saved
  assert(yy_current_buffer);
  include()->yy_current_buffer = yy_current_buffer;          // save the current buffer

	ifstream* newyyin = new ifstream( szIncludeFile );
	assert( newyyin );
	if ( !newyyin->good() ) {
		theGrammar->Error( "Unable to open include file \"%s\"\n", szIncludeFile );
	}

	yy_switch_to_buffer( yy_create_buffer( newyyin, YY_BUF_SIZE ) );
	assert( yy_current_buffer );
	FileLineInfo* fli = new FileLineInfo(
		1,
		currentLine(),
		szIncludeFile,
		yy_current_buffer );
	assert( fli );
	_fileLineInfo.push_back( fli );
	assert(fli->yy_current_buffer);
	assert(include()->yy_current_buffer );
}


bool
strFlexLexer::pop_include()
{
	assert( _fileLineInfo.size() > 0 );
	if ( _fileLineInfo.size() == 1 )
		return false;

	assert(include()->yy_current_buffer );
	yy_delete_buffer( include()->yy_current_buffer );
	_fileLineInfo.pop_back();
	assert(include()->yy_current_buffer );
	yy_switch_to_buffer( include()->yy_current_buffer );

	return true;
}

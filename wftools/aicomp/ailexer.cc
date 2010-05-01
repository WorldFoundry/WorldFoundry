//==============================================================================
// ailexer.cc: Copyright (c) 1996-1999, World Foundry Group  
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
#include <fstream>

#include "ailexer.hp"
//void ai_yyerror( const char* message, ... );
// Don't like this...
#define YY_BUF_SIZE 16384
#include "grammar.hp"

extern bool bVerbose;
                               
                               
aiFlexLexer::aiFlexLexer( istream* arg_yyin, ostream* arg_yyout ) :
		yyFlexLexer( arg_yyin, arg_yyout )
{
}


const char*
aiFlexLexer::filename()
{
	return include()->szFilename;
}


const char*
aiFlexLexer::currentLine()
{
	return include()->_szCurrentLine;
}


const char*
aiFlexLexer::currentLine( const char* szCurrentLine )
{
	return strcpy( include()->_szCurrentLine, szCurrentLine );
}


FileLineInfo*
aiFlexLexer::include()
{
	if ( _fileLineInfo.size() == 0 )
	{
		FileLineInfo fli = FileLineInfo(
			1,
			"",
			theGrammar->filename(),
			yy_current_buffer );
		_fileLineInfo.push_back( fli );
	}
	return _fileLineInfo.end() - 1;
}


void
aiFlexLexer::push_system_include( const char* szIncludeFile )
{
	char szSystemIncludeFile[ _MAX_PATH ];

	char* szVelocityDir = getenv( "WF_DIR" );
	assert( szVelocityDir );
	sprintf( szSystemIncludeFile, "%s/%s", szVelocityDir, szIncludeFile );

	push_include( szSystemIncludeFile );
}


void
aiFlexLexer::push_include( const char* szIncludeFile )
{
	if(bVerbose)
      printf( "Opening file [%s]\n", YYText() );

	ifstream* newyyin = new ifstream( szIncludeFile );
	assert( newyyin );
	if ( !newyyin->good() )
		theGrammar->Error( "Unable to open include file \"%s\"\n", szIncludeFile );

	yy_switch_to_buffer( yy_create_buffer( newyyin, YY_BUF_SIZE ) );

	assert( yy_current_buffer );
	FileLineInfo fli = FileLineInfo(
		1,
		currentLine(),
		szIncludeFile,
		yy_current_buffer );
	_fileLineInfo.push_back( fli );
}


bool
aiFlexLexer::pop_include()
{
	assert( _fileLineInfo.size() > 0 );
	if ( _fileLineInfo.size() == 1 )
		return false;

	yy_delete_buffer( include()->yy_current_buffer );
	_fileLineInfo.pop_back();
	yy_switch_to_buffer( include()->yy_current_buffer );

	return true;
}

#if 0
int
aiFlexLexer::LexerInput( char* buf, int max_size )
	{
	cout << "aiFlexLexer::LexerInput()" << endl;

	if ( yyin->eof() || yyin->fail() )
		return 0;

	yyin->read( buf, max_size );
	//yyin->getline( buf, max_size );

	if ( yyin->bad() )
		return -1;
	else
		return yyin->gcount();
	}
#endif

//==============================================================================
// grammer.cc: Copyright (c) 1996-1999, World Foundry Group  
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
#include <stdarg.h>
#include <fstream>

#include "grammar.hp"
#include "ailexer.hp"
extern aiFlexLexer* theLexer;
extern int programReturnCode;
#include "airun.hp"
extern bool bVerbose;
#include "obj/tokens_tab.h"
#include <version/version.hp>
#include <math/scalar.hp>
#include <recolib/fpath.hp>

aiGrammar* theGrammar;


void
aiGrammar::commit( Function* s )
{
	assert( s );

#if 0
	s = _tblFunction.pop_back();

	if ( s->lineBuffer.size() )
		_program.push_back( s );
#endif
}


Function*
aiGrammar::function()
{
	assert( _tblFunction.size() >= 1 );
	return _tblFunction.back();
}


void
aiGrammar::def_lambda( const char* szDef, const char* szLambda )
{
	// switch to new context to compile code into
	// new Function

#if 0
	for ( int i=0; i<_nFunctions; ++i )
	{
		if ( strcmp( szDef, _subroutines[ i ].name ) == 0 )
			// Redefine
			;
	}

	Function sub;
	strcpy( sub.szName, szDef );
	sub._nTokensInFunction = 0;
	_tblFunction.push_back( sub );
#endif
}


void
aiGrammar::closure()
{
	// pop context
}


void
aiGrammar::_construct( const char* _szInputFile )
{
	assert( _szInputFile );
	assert( *_szInputFile );
	strcpy( _filename, _szInputFile );

	char szDrive[ _MAX_PATH ], szPath[ _MAX_PATH ], szFile[ _MAX_PATH ];
	_splitpath( _szInputFile, szDrive, szPath, szFile, NULL );

	_makepath( szErrorFile, szDrive, szPath, szFile, ".err" );
	error = new ofstream( szErrorFile );
	assert( error );

	szOutputFile[ _MAX_PATH ];
	_makepath( szOutputFile, szDrive, szPath, szFile, ".aib" );
	binout = new ofstream( szOutputFile, ios::out | ios::binary );
	assert( binout );

	char szParseLog[ _MAX_PATH ];
	_makepath( szParseLog, szDrive, szPath, szFile, ".log.htm" );
	_parseLog = new ofstream( szParseLog );
	assert( _parseLog );
	{ // Construct the HTML log
	parseLog() << "<html>" << endl;
	parseLog() << endl;
	parseLog() << "<head>" << endl;
	parseLog() << "<title>" << szParseLog << " [aicomp Parse Log]</title>" << endl;
	parseLog() << "<meta name=\"GENERATOR\" content=\"World Foundry aicomp v" << szVersion << "\">" << endl;

	parseLog() << "</head>" << endl;
	parseLog() << endl;
	parseLog() << "<body>" << endl;

	parseLog() << "<table border=\"1\">" << endl;
	}

	nSymbols = 0;
	nMailboxes = 0;
}


aiGrammar::aiGrammar( const char* _szInputFile, istream* input )
{
	assert( input );

	_construct( _szInputFile );

	theLexer = new aiFlexLexer( input );
	assert( theLexer );
}



#if 0
aiGrammar::aiGrammar( const char* _szInputFile )
{
	_construct( _szInputFile );

	ifstream* inp = new ifstream( _szInputFile, ios::in | ios::nocreate );
	assert( inp );

	theLexer = new aiFlexLexer( inp );
	assert( theLexer );
}
#endif


aiGrammar::~aiGrammar()
{
	int nBytesInErrorFile = error->tellp();
	delete error;
	if ( nBytesInErrorFile == 0 )
		remove( szErrorFile );

	delete theLexer;

	//cout << "size = " << _tblFunction.size() << endl;
//?	assert( _tblFunction.size() == 0 );

//	if ( _program.size() )
	{
	//	int32 nTokensInProgram = _tblFunction.size();
	//	binout->write( (char*)&nTokensInProgram, sizeof( int32 ) );

		vector< Function* >::iterator s;

#if 0
		// Write out the function "jump table"
		// FOURCC offsetInScript
		struct FunctionJumpTableEntry
		{
			uint32 _id;
			uint32 _offset;
		};
		uint32 clCodeOffset = _tblFunction.size() * sizeof( FunctionJumpTableEntry ) / sizeof( uint32 );
		int nTokensInProgram = 0;
		for ( s = _tblFunction.begin(); s != _tblFunction.end(); ++s )
		{
			struct FunctionJumpTableEntry fjte;
			fjte._id = 'cnuf';
			fjte._offset = clCodeOffset + nTokensInProgram;
			binout->write( (char*)( &fjte ), sizeof( fjte ) );
			nTokensInProgram += (*s)->lineBuffer.size();
		}
#endif
		cout << "writing linebuffer to binout at offset " << hex << binout->tellp() << endl;
		for ( s = _tblFunction.begin(); s != _tblFunction.end(); ++s )
		{
			cout << "size = " << (*s)->lineBuffer.size() << endl;

			int32 nTokensInProgram = (*s)->lineBuffer.size();
			binout->write( (char*)&nTokensInProgram, sizeof( int32 ) );

			for ( int i=0; i<(*s)->lineBuffer.size(); ++i )
			{
				//cout << (*s)->lineBuffer[ i ] << endl;
				binout->write( (char*)( &(*s)->lineBuffer[ i ] ), sizeof( int32 ) );
				//binout->write( (char*)( &(*s)->lineBuffer[0] ), (*s)->lineBuffer.size() );
			}
		}

		cout << "writing stringtable  to binout at offset " << hex << binout->tellp() << endl;
		binout->write( _stringTable._strings, _stringTable._cbStrings );
		int32 zero = 0;
		binout->write( (char*)&zero, (4 - _stringTable._cbStrings % 4) % 4 );

//		binout->seekp( 0 );
//		binout->write( (char*)&nTokensInProgram, sizeof( int32 ) );

		delete binout;
	}
//	else
//	{
//		delete binout;
//		remove( szOutputFile );
//	}

	{ // Destruct the log
	parseLog() << "</table>" << endl;
	parseLog() << endl;
	parseLog() << "</body>" << endl;
	parseLog() << "</html>" << endl;
	}
	delete _parseLog;
}

////////////////////////////////////////////////////////////////////////////////

void
aiGrammar::printIncludeList() const
{
	FileLineInfo* fli;
	for ( fli=theLexer->_fileLineInfo.end()-2; fli != theLexer->_fileLineInfo.begin()-1; --fli )
	{
		cout << "\tIncluded from " << fli->szFilename << ":" << fli->nLine << ": " << fli->_szCurrentLine << endl;
	}
}


void
aiGrammar::Error( const char* fmt, ... )
{
	va_list pArg;
	char buffer[ 256 ];
	char* text = buffer;

	va_start( pArg, fmt );
	vsprintf( buffer, fmt, pArg );
	va_end( pArg );

	assert( theGrammar->error );
	cout << "Error: [" << theLexer->filename() << ":" << theLexer->line() << "]: " << theLexer->currentLine() << endl;
	printIncludeList();

	programReturnCode = theLexer->line();
}


void
aiGrammar::Warning( const char* fmt, ... )
{
	va_list pArg;
	char buffer[ 256 ];
	char* text = buffer;

	va_start( pArg, fmt );
	vsprintf( buffer, fmt, pArg );
	va_end( pArg );

	assert( theGrammar->error );
	cout << "Warning: [" << theLexer->filename() << ":" << theLexer->line() << "]: " << theLexer->currentLine() << endl;
	printIncludeList();
}

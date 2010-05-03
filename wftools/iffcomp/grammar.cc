//==============================================================================
// grammar.cc: Copyright (c) 1996-2002, World Foundry Group  
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


#include "grammar.hpp"
#include <pigsys/assert.hp>
#include <stdarg.h>
#include <time.h>
#include <string>
#include <cstdio>
#include <fstream>
using namespace std;
#include "langlex.hpp"
#include "fileline.hpp"

Grammar* theGrammar;
strFlexLexer* theLexer;

//extern bool bVerbose;
extern bool bBinary;

////////////////////////////////////////////////////////////////////////////////

Backpatch::Backpatch( BackpatchType type, int offset )
{
	_type = type;
	_offset = offset;
}

////////////////////////////////////////////////////////////////////////////////

static void
grammarErrorCleanup()
{
	if ( theGrammar && theGrammar->_nErrors )
	{
		delete theGrammar->binout;
		assert( *theGrammar->szOutputFile );
		remove( theGrammar->szOutputFile );
	}
}


void
Grammar::construct( const char* _szInputFile, const char* _szOutputFile )
{
	theGrammar = this;

	binout = NULL;
	strcpy( szOutputFile, _szOutputFile );
	_nErrors = 0;

	atexit( grammarErrorCleanup );

	assert( _szInputFile );
	strcpy( _filename, _szInputFile );
	if ( FILE* fp = fopen( _filename, "rt" ) )
		fclose( fp );
	else
	{
		cerr << "Unable to open input file \"" << _filename << "\"" << endl;
		_nErrors = 1;
		exit( 10 );
	}
#if defined(__WIN__)
	char szDrive[ _MAX_DRIVE ], szPath[ _MAX_PATH ], szFile[ _MAX_FNAME ];
	_splitpath( _szInputFile, szDrive, szPath, szFile, NULL );

	_makepath( szErrorFile, szDrive, szPath, szFile, ".err" );
#else
    strcpy(szErrorFile,_szInputFile);
    strcat(szErrorFile,".err");
#endif
	error = new ofstream( szErrorFile );
	assert( error );

   if(bBinary)
	   binout = new ofstream( _szOutputFile, std::ios::out | std::ios::binary );
   else
	   binout = new ofstream( _szOutputFile, std::ios::out );
	assert( binout );
	if ( !binout->good() )
	{
		cerr << "Unable to open output file \"" << _szOutputFile << "\" for writing" << endl;
		_nErrors = 1;
		exit( 10 );
	}

	if ( bBinary )
		_iff = new IffWriterBinary( *binout );
	else
		_iff = new IffWriterText( *binout );
	assert( _iff );

	_level = 0;

	//if ( bVerbose )
	//	_iff->log( cout );

	State defaultState;
	defaultState.sizeOverride( 1 );
	size_specifier default_fp = { 1, 15, 16 };
	defaultState.precision( default_fp );
	vecState.push_back( defaultState );
}


Grammar::Grammar( const char* _szInputFile, const char* _szOutputFile )
{
	construct( _szInputFile, _szOutputFile );

	_inp = new ifstream( _szInputFile );
	assert( _inp );

	theLexer = new strFlexLexer( _inp );
	assert( theLexer );
}


Grammar::~Grammar()
{
	vector< Backpatch* >::const_iterator iChunk;
	for ( iChunk = _backpatchSizeOffset.begin(); iChunk != _backpatchSizeOffset.end(); ++iChunk )
	{
//         cout << "Grammar::~Grammar" << endl;
//         cout << (*iChunk)->szChunkIdentifier << endl;
//         cout << (*iChunk)->pos << endl;
//         cout << (*iChunk)->_type << endl;

        // print all available symbols
//        cout << "iffwriter = " << *_iff << endl;

		const ChunkSizeBackpatch* cs = _iff->findSymbol( (*iChunk)->szChunkIdentifier );
		if ( cs )
		{
			binout->seekp( (*iChunk)->pos );
//          if ( (*iChunk)->_offset )
//             cout << "_offset = " << (*iChunk)->_offset << endl;
			long val = (*iChunk)->_type == Backpatch::TYPE_OFFSETOF ? cs->GetPos()-4 : cs->GetSize();
//			cout << "val = " << val;
			val += (*iChunk)->_offset;

         //assert(val >= 0);

//			cout << "\t_offset = " << (*iChunk)->_offset << "\tval + _offset = " << val << endl;
			binout->write( (char*)&val, 4 );
		}
		else
		{
			Error( "Unable to find chunk ID \"%s\"", (*iChunk)->szChunkIdentifier );
		}
	}

	assert( _level == 0 );

	assert( vecState.size() == 1 );
	vecState.pop_back();

	assert( _iff );
	delete _iff;

	assert( _inp );
	delete _inp;

	int nBytesInErrorFile = error->tellp();
	delete error;
	if ( nBytesInErrorFile == 0 )
		remove( szErrorFile );

	delete theLexer;

	delete binout;
	grammarErrorCleanup();		// includes delete binout;
}


void
Grammar::printIncludeList() const
{
	//FileLineInfo* * ifli;
	std::vector< FileLineInfo* >::const_iterator ifli;

	for ( ifli=theLexer->_fileLineInfo.end()-2; ifli != theLexer->_fileLineInfo.begin()-1; --ifli )
	{
		FileLineInfo* fli = *ifli;
		cerr << "\tIncluded from " << fli->szFilename << ":" << fli->nLine << ": " << fli->_szCurrentLine << endl;
	}
}


void
Grammar::Error( const char* fmt, ... )
{
	va_list pArg;
	char buffer[ 256 ];

	va_start( pArg, fmt );
	vsprintf( buffer, fmt, pArg );
	va_end( pArg );

	++_nErrors;

	assert( theGrammar->error );
	// the error message itself
	cerr << "Error: " << buffer << endl;
	*theGrammar->error << "Error: " << buffer << endl;

	// the offending line from the input file
	cerr << "[" << theLexer->filename() << ":" << theLexer->line() << "]: " << theLexer->currentLine() << endl;
	*theGrammar->error << "[" << theLexer->filename() << ":" << theLexer->line() << "]: " << theLexer->currentLine() << endl;
	printIncludeList();
}


void
Grammar::Warning( const char* fmt, ... )
{
	va_list pArg;
	char buffer[ 256 ];

	va_start( pArg, fmt );
	vsprintf( buffer, fmt, pArg );
	va_end( pArg );

	assert( theGrammar->error );

	// the warning message itself
	cerr << "Warning: " << buffer << endl;
	*theGrammar->error << "Warning: " << buffer << endl;

	// the offending line from the input file
	cerr << "[" << theLexer->filename() << ":" << theLexer->line() << "]: " << theLexer->currentLine() << endl;
	*theGrammar->error << "[" << theLexer->filename() << ":" << theLexer->line() << "]: " << theLexer->currentLine() << endl;
	printIncludeList();
}


int
Grammar::yyparse()
{
	::yyparse();
	return _nErrors ? 10 : 0;
}

//==============================================================================
// grammar.cc: Copyright (c) 1996-2002, 2026, World Foundry Group
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
#include "langlex.hpp"
#include "fileline.hpp"
#include "lang.tab.hh"

#include <pigsys/assert.hp>

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>


extern bool bBinary;

////////////////////////////////////////////////////////////////////////////////

Backpatch::Backpatch( BackpatchType type, int offset )
    : _type( type ), pos( 0 ), _offset( offset )
{
}

////////////////////////////////////////////////////////////////////////////////

void
Grammar::construct( const char* _szInputFile, const char* _szOutputFile )
{
    binout  = nullptr;
    _lex    = nullptr;
    _nErrors = 0;

    std::strncpy( szOutputFile, _szOutputFile, PATH_MAX - 1 );
    szOutputFile[ PATH_MAX - 1 ] = '\0';

    assert( _szInputFile );
    std::strncpy( _filename, _szInputFile, PATH_MAX - 1 );
    _filename[ PATH_MAX - 1 ] = '\0';

    if ( std::FILE* fp = std::fopen( _filename, "r" ) )
        std::fclose( fp );
    else
    {
        std::cerr << "Unable to open input file \"" << _filename << "\"" << std::endl;
        _nErrors = 1;
        std::exit( 10 );
    }

    std::strncpy( szErrorFile, _szInputFile, PATH_MAX - 1 );
    szErrorFile[ PATH_MAX - 1 ] = '\0';
    std::strncat( szErrorFile, ".err", PATH_MAX - std::strlen( szErrorFile ) - 1 );

    error = new std::ofstream( szErrorFile );
    assert( error );

    if ( bBinary )
        binout = new std::ofstream( _szOutputFile, std::ios::out | std::ios::binary );
    else
        binout = new std::ofstream( _szOutputFile, std::ios::out );
    assert( binout );
    if ( !binout->good() )
    {
        std::cerr << "Unable to open output file \"" << _szOutputFile
                  << "\" for writing" << std::endl;
        _nErrors = 1;
        std::exit( 10 );
    }

    if ( bBinary )
        _iff = new IffWriterBinary( *binout );
    else
        _iff = new IffWriterText( *binout );
    assert( _iff );

    _level = 0;

    State defaultState;
    defaultState.sizeOverride( 1 );
    size_specifier default_fp = { 1, 15, 16 };
    defaultState.precision( default_fp );
    vecState.push_back( defaultState );
}


Grammar::Grammar( const char* _szInputFile, const char* _szOutputFile )
{
    construct( _szInputFile, _szOutputFile );
}


Grammar::~Grammar()
{
    // Resolve any outstanding sizeof/offsetof backpatches.
    for ( Backpatch* bp : _backpatchSizeOffset )
    {
        const ChunkSizeBackpatch* cs = _iff->findSymbol( bp->szChunkIdentifier.c_str() );
        if ( cs )
        {
            binout->seekp( bp->pos );
            int32_t val = static_cast< int32_t >(
                ( bp->_type == Backpatch::TYPE_OFFSETOF )
                    ? cs->GetPos() - 4
                    : cs->GetSize() );
            val += bp->_offset;
            binout->write( reinterpret_cast< const char* >( &val ), 4 );
        }
        else
        {
            Error( "Unable to find chunk ID \"%s\"", bp->szChunkIdentifier.c_str() );
        }
        delete bp;
    }
    _backpatchSizeOffset.clear();

    assert( _level == 0 );

    assert( vecState.size() == 1 );
    vecState.pop_back();

    delete _iff;
    _iff = nullptr;

    int nBytesInErrorFile = error ? static_cast< int >( error->tellp() ) : 0;
    delete error;
    error = nullptr;
    if ( nBytesInErrorFile == 0 )
        std::remove( szErrorFile );

    // If parsing recorded errors, scrap the (now-garbage) output file.
    const bool parseFailed = ( _nErrors != 0 );
    delete binout;
    binout = nullptr;
    if ( parseFailed && *szOutputFile )
        std::remove( szOutputFile );
}


void
Grammar::printIncludeList() const
{
    if ( !_lex )
        return;

    const auto& info = _lex->_fileLineInfo;
    if ( info.size() < 2 )
        return;

    for ( auto ifli = info.end() - 2; ifli != info.begin() - 1; --ifli )
    {
        FileLineInfo* fli = *ifli;
        std::cerr << "\tIncluded from " << fli->szFilename
                  << ":" << fli->nLine
                  << ": " << fli->_szCurrentLine << std::endl;
    }
}


void
Grammar::Error( const char* fmt, ... )
{
    va_list pArg;
    char buffer[ 256 ];

    va_start( pArg, fmt );
    std::vsnprintf( buffer, sizeof buffer, fmt, pArg );
    va_end( pArg );

    ++_nErrors;

    assert( error );
    std::cerr << "Error: " << buffer << std::endl;
    *error    << "Error: " << buffer << std::endl;

    if ( _lex )
    {
        std::cerr << "[" << _lex->filename() << ":" << _lex->line() << "]: "
                  << ( _lex->currentLine() ? _lex->currentLine() : "" ) << std::endl;
        *error << "[" << _lex->filename() << ":" << _lex->line() << "]: "
               << ( _lex->currentLine() ? _lex->currentLine() : "" ) << std::endl;
        printIncludeList();
    }
}


void
Grammar::Warning( const char* fmt, ... )
{
    va_list pArg;
    char buffer[ 256 ];

    va_start( pArg, fmt );
    std::vsnprintf( buffer, sizeof buffer, fmt, pArg );
    va_end( pArg );

    assert( error );
    std::cerr << "Warning: " << buffer << std::endl;
    *error    << "Warning: " << buffer << std::endl;

    if ( _lex )
    {
        std::cerr << "[" << _lex->filename() << ":" << _lex->line() << "]: "
                  << ( _lex->currentLine() ? _lex->currentLine() : "" ) << std::endl;
        *error << "[" << _lex->filename() << ":" << _lex->line() << "]: "
               << ( _lex->currentLine() ? _lex->currentLine() : "" ) << std::endl;
        printIncludeList();
    }
}


int
Grammar::yyparse()
{
    LangLexer lex( *this, _filename );
    _lex = &lex;

    yy::LangParser parser( *this, lex );
    parser.parse();

    _lex = nullptr;
    return _nErrors ? 10 : 0;
}

//==============================================================================
// langlex.cc: Copyright (c) 1996-1999, 2026, World Foundry Group
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

#include "langlex.hpp"
#include "grammar.hpp"
#include "fileline.hpp"

#include <pigsys/assert.hp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Flex-runtime state and buffer-stack API, forward-declared so we don't have to
// pull in the entire scanner .cc. Flex emits these as external-linkage symbols.
struct yy_buffer_state;
typedef yy_buffer_state* YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_create_buffer( std::FILE*, int );
extern void yypush_buffer_state( YY_BUFFER_STATE );
extern void yypop_buffer_state();
extern std::FILE* yyin;

static constexpr int kYyBufSize = 16384;


LangLexer::LangLexer( Grammar& g, const char* szInputFile )
    : _g( g ), _rootFile( nullptr )
{
    _rootFile = std::fopen( szInputFile, "r" );
    if ( !_rootFile )
    {
        std::perror( szInputFile );
        std::exit( 10 );
    }
    yyin = _rootFile;

    FileLineInfo* fli = new FileLineInfo( 1, "", szInputFile );
    assert( fli );
    _fileLineInfo.push_back( fli );
}


LangLexer::~LangLexer()
{
    for ( FileLineInfo* fli : _fileLineInfo )
        delete fli;
    _fileLineInfo.clear();

    if ( _rootFile )
    {
        std::fclose( _rootFile );
        _rootFile = nullptr;
    }
}


const char*
LangLexer::filename()
{
    return include()->szFilename;
}


const char*
LangLexer::currentLine()
{
    return include()->_szCurrentLine;
}


const char*
LangLexer::currentLine( const char* szCurrentLine )
{
    return include()->_szCurrentLine = const_cast< char* >( szCurrentLine );
}


FileLineInfo*
LangLexer::include()
{
    if ( _fileLineInfo.empty() )
    {
        FileLineInfo* fli = new FileLineInfo( 1, "", _g.filename() );
        assert( fli );
        _fileLineInfo.push_back( fli );
    }
    return _fileLineInfo.back();
}


void
LangLexer::push_system_include( const char* szIncludeFile )
{
    const char* szWorldFoundryDir = std::getenv( "WF_DIR" );
    assert( szWorldFoundryDir );

    std::string full = std::string( szWorldFoundryDir ) + "/" + szIncludeFile;
    push_include( full.c_str() );
}


void
LangLexer::push_include( const char* szIncludeFile )
{
    std::FILE* fp = std::fopen( szIncludeFile, "r" );
    if ( !fp )
    {
        _g.Error( "Unable to open include file \"%s\"", szIncludeFile );
        return;
    }

    yypush_buffer_state( yy_create_buffer( fp, kYyBufSize ) );

    FileLineInfo* fli = new FileLineInfo( 1, currentLine(), szIncludeFile );
    assert( fli );
    _fileLineInfo.push_back( fli );
}


bool
LangLexer::pop_include()
{
    assert( !_fileLineInfo.empty() );
    if ( _fileLineInfo.size() == 1 )
        return false;

    delete _fileLineInfo.back();
    _fileLineInfo.pop_back();
    yypop_buffer_state();

    return true;
}

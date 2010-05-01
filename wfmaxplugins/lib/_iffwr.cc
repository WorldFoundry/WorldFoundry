//=============================================================================
// _iffwr.cc
// Copyright 1997,99 World Foundry Group. 
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

// ===========================================================================
// History:
// ?? ??? ??	WBNIV	Created
//============================================================================

#include "iffwrite.hp"
#include <time.h>
#include <string>
#include <strmnull.hp>
//#include <fstream>
//#include <iomanip>
//using namespace std;
//DEL?#include <minmax.h>

//DELvoid* LoadBinaryFile( const char* szFilename, unsigned long &sizeOfFile );

IffWriterNull theNullIffWriter;

#if SW_DBSTREAM > 0
static nullstream cnull_iff;
#endif

_IffWriter::_IffWriter()
{
	DBSTREAM1( log( cnull_iff ); )
	_fillChar = 0;

	ChunkSizeBackpatch* cs = new ChunkSizeBackpatch;
	assert( cs );
	strcpy( cs->szID, "" );
	chunkSize.push_back( cs );
}


_IffWriter::~_IffWriter()
{
//	assert( chunkSize.size() == 1 );
	while ( chunkSize.size() )
		chunkSize.pop_back();
}


unsigned char
_IffWriter::fillChar() const
{
	return _fillChar;
}


void
_IffWriter::fillChar( unsigned char fillChar )
{
	_fillChar = fillChar;
}


const ChunkSizeBackpatch*
_IffWriter::findSymbol( const char* szSymbolName ) const
{
	vector< ChunkSizeBackpatch* >::const_iterator iSym;
	for ( iSym = _chunkIdList.begin(); iSym != _chunkIdList.end(); ++iSym )
	{
		if ( strcmp( szSymbolName, (*iSym)->szID ) == 0 )
			return *iSym;
	}

	return NULL;
}


ID
_IffWriter::chunkName() const
{
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	return cs->id;
}


void
_IffWriter::enterChunk( const ID& id )
{
	log() << endl << tab( chunkSize.size() ) << "{ " << id << ' ';

	ChunkSizeBackpatch* csParent = chunkSize.back();
	assert( csParent );

	ChunkSizeBackpatch* cs = new ChunkSizeBackpatch;
	assert( cs );
	cs->id = id;

//	s << csParent->szID << "::" << id.name();
	strcpy( cs->szID, csParent->szID );
	strcat( cs->szID, "::'" );
	strcat( cs->szID, id.name() );
	strcat( cs->szID, "'" );
	//cout << "Entering chunk " << cs->szID << endl;
	chunkSize.push_back( cs );
}


ChunkSizeBackpatch*
_IffWriter::exitChunk()
{
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	chunkSize.pop_back();

	_chunkIdList.push_back( cs );

	log() << endl << tab( chunkSize.size() ) << '}';

	return cs;
}


ChunkSizeBackpatch*
_IffWriter::exitChunk( const ID& id )
{
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
#if !defined( NDEBUG )
	if ( !( id == cs->id ) )
		cerr << "Ending chunk " << id << " but was expecting " << cs->id << endl;
// 	assert( id == cs->id );
#endif

	return exitChunk();
}


void
IffWriterText::out_file( const File& file )
{
	assert(0);
#if 0
	log() << '"' << file.filename() << "\" ";

	int32 cbFile;
	void* ptr = LoadBinaryFile( file.filename(), cbFile );
	if ( !ptr )
		throw;	//return false;
	assert( ptr );

	log() << "size=" << cbFile << ' ';

	unsigned long startPos = max( 0, file.startPos() );
	startPos = min( startPos, cbFile );
	unsigned long len = min( cbFile - startPos, file.length() );

	out_mem( (char*)ptr + startPos, len );
	free( ptr );
#endif
}


void
IffWriterBinary::out_file( const File& file )
{
	log() << '"' << file.filename() << "\" ";

	assert(0);
#if 0
	int32 cbFile;
	void* ptr = LoadBinaryFile( file.filename(), cbFile );
	if ( !ptr )
		throw;	//return false;
	assert( ptr );

	log() << "size=" << cbFile << ' ';

	//log() << '$' << hex << assetId << dec << ' ';
	//out_id( ID( assetId ) );

	unsigned long startPos = max( 0, file.startPos() );
	startPos = min( startPos, cbFile );
	unsigned long len = min( cbFile - startPos, file.length() );

	out_mem( (char*)ptr + startPos, len );
	free( ptr );
#endif
}


#if 0
void
_IffWriter::out_string( const char* str, int cbStr )
{
	log() << '"' << str << "\" ";
	//cout << str << " len=" << strlen( str ) << " maxlen=" << cbStr << endl;
	assert( strlen( str ) + 1 <= cbStr );

	out_string( str );		// includes terminating \0

	int cbPad = cbStr - strlen( str ) - 1;
	for ( int i=0; i<cbPad; ++i )
		out_int8( 0 );
}
#endif


void
IffWriterText::out_timestamp( const Timestamp& ts )
{
	char szTimestamp[ 100 ];
	strcpy( szTimestamp, asctime( ts.pTime ) );
	*( szTimestamp + strlen( szTimestamp ) - 1 ) = '\0';
	log() << "Timestamp is " << hex << ts._localtimestamp << dec << " (" << szTimestamp << ')';
	out_int32( ts._localtimestamp );
}


void
IffWriterBinary::out_timestamp( const Timestamp& ts )
{
	char szTimestamp[ 100 ];
	strcpy( szTimestamp, asctime( ts.pTime ) );
	*( szTimestamp + strlen( szTimestamp ) - 1 ) = '\0';
	log() << "Timestamp is " << hex << ts._localtimestamp << dec << " (" << szTimestamp << ')';
	out_int32( ts._localtimestamp );
}


void
IffWriterText::alignFunction( int cbAlign )
{
	*_out << ".ALIGN " << cbAlign << ' ';
}


void
IffWriterBinary::alignFunction( int cbAlign )
{
	assert( _out );
	streampos pos = _out->tellp() % cbAlign;
	if ( pos )
		pos = cbAlign-pos;
	unsigned char fc = fillChar();
	for ( int i=0; i<pos; ++i )
		out_int8( fc );
}

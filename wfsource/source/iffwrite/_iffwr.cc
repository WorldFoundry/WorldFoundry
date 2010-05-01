//=============================================================================
// _iffwr.cc
// Copyright 1997,1999,2000,2001,2003 World Foundry Group. 
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

#include <cpplib/libstrm.hp>
//#include <cpplib/strmnull.hp>
//#include <pigsys/pigsys.hp>
//#include <math/scalar.hp>
#include <iffwrite/iffwrite.hp>
#include <loadfile/loadfile.hp>
#include <time.h>
//#include <string>
//#include <fstream>
//#include <iomanip>
//DEL?#include <minmax.h>

#include <pigsys/minmax.hp>
#include <cpplib/stdstrm.hp>
#include <cpplib/strmnull.hp>

//#include <loadfile/loadfile.hp>
//DELvoid* LoadBinaryFile( const char* szFilename, unsigned long &sizeOfFile );

IffWriterNull theNullIffWriter;

#if SW_DBSTREAM > 0
CREATENULLSTREAM(cnull_iff);
#endif

_IffWriter::_IffWriter()
{
	_fillChar = 0;

	ChunkSizeBackpatch* cs = new ChunkSizeBackpatch();
	assert( cs );
	//strcpy( cs->szID, "" );
	chunkSize.push_back( cs );
}

_IffWriter::~_IffWriter()
{
//	assert( chunkSize.size() == 1 );
	std::vector< ChunkSizeBackpatch* >::iterator csi = chunkSize.begin();

	while(csi != chunkSize.end())
	{
		delete(*csi++);
	}

	csi = _chunkIdList.begin();
	while(csi != _chunkIdList.end())
	{
		delete(*csi++);
	}


}

#if DO_IOSTREAMS

std::ostream& operator<<(std::ostream& out, const _IffWriter& iffw)  
{
    out << "_IFFWriter dump: " << std::endl;
	unsigned int index;    
    for(index=0;index < iffw.chunkSize.size(); index++)
        out << "chunksize[" << index << "] = " << *iffw.chunkSize[index] << std::endl;

    out << "filename = " << iffw._filename << std::endl;
    out << "fillchar = <" << iffw._fillChar << ">" << std::endl;

    for(index=0;index < iffw._chunkIdList.size(); index++)
        out << "chunkIdList[" << index << "] = " << *iffw._chunkIdList[index] << std::endl;
    return out;
}

#endif


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
	std::vector< ChunkSizeBackpatch* >::const_iterator iSym;
	for ( iSym = _chunkIdList.begin(); iSym != _chunkIdList.end(); ++iSym )
	{
		if ( strcmp( szSymbolName, (*iSym)->GetName() ) == 0 )
			return *iSym;
	}

	return NULL;
}


ID
_IffWriter::chunkName() const
{
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	return cs->GetID();
}


void
_IffWriter::enterChunk( const ID& id )
{
	DBSTREAM1( ciff  << std::endl << tab( chunkSize.size() ) << "{ " << id << ' ' << std::endl; )

	ChunkSizeBackpatch* csParent = chunkSize.back();
	assert( csParent );


    char name[256];
//	s << csParent->szID << "::" << id.name();
	//strcpy( name, csParent->GetID().name() );
	strcpy( name, csParent->GetName() );
	strcat( name, "::'" );
	strcat( name, id.name() );
	strcat( name, "'" );
    //std::cout << "_IffWriter::enterChunk: name = " << name << std::endl;

	ChunkSizeBackpatch* cs = new ChunkSizeBackpatch(id,int(_out->tellp())+4,-8,name);
	assert( cs );
//     cs->id = id;
//     cs->size = -8;
//     cs->pos = int(_out->tellp())+4;     // + chunkId
    
	//std::cout << "Entering chunk " << cs->szID << std::endl;
	chunkSize.push_back( cs );
}


ChunkSizeBackpatch*
_IffWriter::exitChunk()
{
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	chunkSize.pop_back();

	_chunkIdList.push_back( cs );

	DBSTREAM1( ciff << std::endl << tab( chunkSize.size() ) << '}'<< std::endl; )
	return cs;
}


ChunkSizeBackpatch*
_IffWriter::exitChunk( const ID& id )
{
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
#if !defined( NDEBUG )
	if ( !( id == cs->GetID() ) )
		std::cerr << "Ending chunk " << id << " but was expecting " << cs->GetID() << std::endl;
// 	assert( id == cs->id );
#endif

	return exitChunk();
}


void
IffWriterText::out_file( const File& file )
{
	DBSTREAM1( ciff  << "IFFWriterText::out_file: filename = " << '"' << file.filename() << "\" "; )

	int32 cbFile;
	void* ptr = LoadBinaryFile( file.filename(), cbFile );
	if ( !ptr )
    {
        DBSTREAM1(cerror << "can't open file " << file.filename() << std::endl; )
		throw;	//return false;
    }
	assert( ptr );

	DBSTREAM1( ciff << "size=" << cbFile << ' '; )

	int32 startPos = max( int32(0), int32(file.startPos()) );
	startPos = min( startPos, cbFile );
	unsigned long len = min( cbFile - startPos, file.length() );

    AssertMsg(len > 0,"length of file [" << file.filename() << "] is 0"); 

	out_mem( (char*)ptr + startPos, len );
	free( ptr );
}


void
IffWriterBinary::out_file( const File& file )
{
	DBSTREAM1( ciff <<  "IFFWriterBinary::out_file: filename = " <<'"' << file.filename() << "\" "; )

	int32 cbFile;
	void* ptr = LoadBinaryFile( file.filename(), cbFile );
	if ( !ptr )
    {
        DBSTREAM1( cerror << "can't open file " << file.filename() << std::endl; )
		throw;	//return false;

    }
	assert( ptr );

	DBSTREAM1( ciff << "size=" << cbFile << ' '; )

	//DBSTREAM1( ciff << '$' << hex << assetId << dec << ' '; )
	//out_id( ID( assetId ) );

	int32 startPos = max( int32(0), int32(file.startPos()) );
	startPos = min( startPos, cbFile );
	int32 len =   cbFile - startPos;
	if(file.length() >= 0)
		len = min(len, file.length() );

    AssertMsg(len > 0,"length of file [" << file.filename() << "] is 0"); 
	out_mem( (char*)ptr + startPos, len );
	free( ptr );
}


#if 0
void
_IffWriter::out_string( const char* str, int cbStr )
{
	DBSTREAM1( ciff << '"' << str << "\" "; )
	//std::cout << str << " len=" << strlen( str ) << " maxlen=" << cbStr << std::endl;
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
	DBSTREAM1( ciff << "Timestamp is " << std::hex << ts._localtimestamp << std::dec << " (" << szTimestamp << ')'; )
	out_int32( ts._localtimestamp );
}


void
IffWriterBinary::out_timestamp( const Timestamp& ts )
{
	char szTimestamp[ 100 ];
	strcpy( szTimestamp, asctime( ts.pTime ) );
	*( szTimestamp + strlen( szTimestamp ) - 1 ) = '\0';
	DBSTREAM1( ciff << "Timestamp is " << std::hex << ts._localtimestamp << std::dec << " (" << szTimestamp << ')'; )
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
	std::streampos pos = _out->tellp() % cbAlign;
	if ( pos )
		pos = cbAlign-pos;
	unsigned char fc = fillChar();
	for ( int i=0; i<pos; ++i )
		out_int8( fc );
}

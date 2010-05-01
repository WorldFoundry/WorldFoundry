//=============================================================================
// binary.cc
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
// 28 Feb 98	WBNIV	Created [from iffwrite.cc]
//============================================================================

#include "global.hp"
#include "iffwrite.hp"
#include <time.h>
#include <string>
#include <fstream.h>
#include <iomanip.h>
#include <ostream.h>
//#include <iomanip>
//using namespace std;

inline int round( const int n, const int align )
{
	return ( (n+align-1) / align) * align;
}


#if 0
void
IffWriterBinary::out_strstream( strstream& str )
{
	assert( _out );
	unsigned char* sz = (unsigned char*)str.str();

	int nChars = strlen( str.str() );	// not actually what I want
	for ( int i=0; i<nChars; ++i )
		*this << *sz++;
}
#endif


void
IffWriterBinary::out_int8( unsigned char i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
}


void
IffWriterBinary::out_int16( unsigned short i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
}


void
IffWriterBinary::out_int32( unsigned long i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
}

#if 0
void
IffWriterBinary::out_scalar( const Scalar& s )
{
	long i = s.AsLong();

	assert( _out );
	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
}
#endif


static char szSize[] = { '!', 'b', 'w', '!', 'l' };

void
IffWriterBinary::align( int cbAlign )
{
	static unsigned long zero = 0UL;

	assert( 0 <= cbAlign && cbAlign <= 4 );
	assert( _out );
	streampos pos = _out->tellp() % 4;
	_out->write( (const char*)&zero, pos ? 4-pos : 0 );
}


void
IffWriterBinary::out_id( const ID& id )
{
	assert( _out );
	align( 4 );
	out_int32( id() );
}


char*
IffWriterBinary::translate_escape_codes( char* _str, int& len )
{
	char* str = strchr( _str, '\\' );
	while ( str )
	{
		char* pNextChar = str+1;
		if ( *pNextChar == 't' )
		{
			*str = '\t';
			++str;
			strcpy( str, str+1 );
		}
		else if ( *pNextChar == 'n' )
		{
			*str = '\n';
			++str;
			strcpy( str, str+1 );
		}
		else if ( *pNextChar == '\\' )
		{
			++str;
			strcpy( str, str+1 );
		}
		else if ( '0' <= *pNextChar && *pNextChar <= '9' )
		{
			int i = atoi( pNextChar );
			if ( i > 255 )
				i &= 0xFF;
			*str = atoi( pNextChar );
			++str;

			while ( isdigit( *pNextChar ) )
				++pNextChar;
			strcpy( str, pNextChar );
		}

		cout << '[' << str << ']';
		str = strchr( str, '\\' );
	}

	len = str - _str;

	return _str;
}



#if 0
void
IffWriterBinary::out_string( const char* str )
{
	log() << "\"" << str << "\" ";
	assert( _out );

	char* sz = strdup( str );
	assert( sz );
	int len;
	translate_escape_codes( sz, len );
	_out->write( sz, strlen( sz ) + 1 );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += strlen( sz ) + 1;

	free( sz );
}
#endif


void
IffWriterBinary::out_string( const string& str )
{
	log() << "\"" << str.c_str() << "\" ";
	assert( _out );

	char* sz = strdup( str.c_str() );
	assert( sz );
	int len;
	translate_escape_codes( sz, len );
	_out->write( sz, strlen( sz ) + 1 );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += strlen( sz ) + 1;

	free( sz );
}


void
IffWriterBinary::out_string_continue( const string& str )
{
	// backup single character
	assert( _out );
	_out->seekp( -1, ios::cur );

	out_string( str );
}


void
IffWriterBinary::out_mem( void* ptr, size_t size )
{
	assert( _out );
	assert( ptr );

	_out->write( (const char*)ptr, size );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += size;
}


#if 0
void
IffWriterBinary::out_int( long i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
}
#endif


void
IffWriterBinary::out_comment( const Comment& comment )
{
}


void
IffWriterBinary::enterChunk( const ID& id )
{
	assert( _out );

	_IffWriter::enterChunk( id );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size = -8;
	cs->pos = int(_out->tellp())+4;		// + chunkId

	*this << id;
	*this << ~0UL;
	//[new]out_int32( ~0 );
}


ChunkSizeBackpatch*
IffWriterBinary::exitChunk()
{
	assert( _out );

	ChunkSizeBackpatch* cs = _IffWriter::exitChunk();
	assert( cs );

	int pos = _out->tellp();

	_out->seekp( cs->pos, ios::beg );
	assert( sizeof( cs->size ) == 4 );
	_out->write( (char const*)&( cs->size ), sizeof( cs->size ) );
	_out->seekp( pos, ios::beg );

	if ( !chunkSize.empty() )
	{
		ChunkSizeBackpatch* csParent = chunkSize.back();
		assert( cs );
		int remainder = cs->size % 4;
		csParent->size += cs->size;
		csParent->size += remainder ? 4 - remainder : 0;
		csParent->size += 4 + 4;		// ID, size
	}

	align( 4 );

	return cs;
}


IffWriterBinary::IffWriterBinary( ostream& out ) : _IffWriter()
{
	//assert( _log );

	_out = &out;
	_out->setf( ios::showpoint );
}


IffWriterBinary::~IffWriterBinary()
{
	assert( _out );
}


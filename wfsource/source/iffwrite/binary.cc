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

#include <pigsys/assert.hp>
//#include <math/scalar.hp>
#include <iffwrite/iffwrite.hp>
//#include <time.h>
//#include <string>
//#include <fstream>
//#include <iomanip>

inline int round( const int n, const int align )
{
	return ( (n+align-1) / align) * align;
}


#if 0
void
IffWriterBinary::out_strstream( std::strstream& str )
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
	cs->AddToSize(sizeof( i ));
}


void
IffWriterBinary::out_int16( unsigned short i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(sizeof( i ));
}


#if 0
void
IffWriterBinary::out_int32( unsigned long i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(sizeof( i ));
}
#endif

void
IffWriterBinary::out_int32( int32 i )
{
	assert( _out );
	long before = long(_out->tellp());
	DBSTREAM1( ciff << "IFFWriterBinary::out_int32: value = " << i << ", streampos before = " << before;)

	assert(_out->good());
	_out->write( (char const*)&i, sizeof( i ) );
	assert(_out->good());
	DBSTREAM1( ciff << ", sizeof(int32) = " << sizeof(i) << ", streampos after = " << _out->tellp() << std::endl;)

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(sizeof( i ));
	assert(long(_out->tellp()) == (before+sizeof(i)));
}


// kts: should the serialization code be in math, or here?
void
IffWriterBinary::out_scalar( const Scalar& s )
{
	DBSTREAM1( ciff << "IFFWriterBinary::out_scalar: value = " << s << std::endl; )

   // kts note: this assumes that scalar is a concrete class with no pointers
   int size = sizeof(Scalar);
   const char* data = (const char*)&s;

	assert( _out );
   _out->write(data,size);

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(sizeof( size ));
}

// compiler say's this is "defined but not used."
//static char szSize[] = { '!', 'b', 'w', '!', 'l' };

void
IffWriterBinary::align( int cbAlign )
{
	static unsigned long zero = 0UL;
	DBSTREAM1( ciff << "IFFWriterBinary::align: streampos before: " <<  _out->tellp(); )

	assert( 0 <= cbAlign && cbAlign <= 4 );
	assert( _out );
	std::streampos pos = _out->tellp() % 4;
	_out->write( (const char*)&zero, pos ? 4-pos : 0 );
	DBSTREAM1( ciff << ", streampos after: " <<  _out->tellp(); )
}


void
IffWriterBinary::out_id( const ID& id )
{
	DBSTREAM1( ciff << "IFFWriterBinary::out_id: value = " << id << std::endl; )

	assert( _out );
	align( 4 );
	out_int32( id() );
}

#pragma message ("KTS " __FILE__ ": this shouldn't be here, it should be in cpplib as a standard call, called by iffcomp")
static char*
translate_escape_codes( char* _str )
{
	char* str = strchr( _str, '\\' );
	while ( str && strlen(str) > 1)
	{
		assert(strlen(str) > 1);
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
		else if ( *pNextChar == '"' )
		{
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
		else
		{
			AssertMsg(0,"Invalid Escape code " << str);
		}	

		//cout << "IFFWriterBinary::TranslateEscapeCodes[" << str << ']' << endl;
		str = strchr( str, '\\' );
	}
	return _str;
}



#if 0
void
IffWriterBinary::out_string( const char* str )
{
	DBSTREAM1( ciff << "\"" << str << "\" "; )
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
IffWriterBinary::out_string( const std::string& str )
{
	DBSTREAM1( ciff << "IffWriterBinary::out_string: \"" << str.c_str() << "\" "; )
	assert( _out );

	char* sz = strdup( str.c_str() );
	//cout << "IWB::os: translate escape codes
	assert( sz );
	translate_escape_codes( sz);
	_out->write( sz, strlen( sz ) + 1 );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(strlen( sz ) + 1);

	free( sz );
}


void
IffWriterBinary::out_string_continue( const std::string& str )
{
	// backup single character (since we already 0 terminated the string)
	assert( _out );
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->SubtractFromSize(1);
	_out->seekp( -1, std::ios::cur );
	out_string( str );
}


void
IffWriterBinary::out_mem( void* ptr, size_t size )
{
	assert( _out );
	assert( ptr );
    assert(size > 0);
    assert(size < 10000000);     // kts arbitrary

	_out->write( (const char*)ptr, size );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(size);
}


#if 0
void
IffWriterBinary::out_int( long i )
{
	assert( _out );

	_out->write( (char const*)&i, sizeof( i ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(sizeof( i ));
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

	DBSTREAM1( ciff << "entering chunk " << id << ", file position = " << _out->tellp() << std::endl; )

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );

	*this << id;
	*this << ~0UL;
	//[new]out_int32( ~0 );
}


ChunkSizeBackpatch*
IffWriterBinary::exitChunk()
{
	assert( _out );

	DBSTREAM1( ciff << "IffWriterBinary::exitChunk:" << std::endl; )

	ChunkSizeBackpatch* cs = _IffWriter::exitChunk();
	assert( cs );

	int pos = _out->tellp();

	//AssertMsg(pos == (cs->pos + cs->size + sizeof(cs->size)),"Current Position = " << pos << ", starting position = " << cs->pos << ", size = " << cs->size);			//kts 12/17/99 2:23PM
	AssertMsg(pos == (cs->GetPos() + cs->GetSize() + 4),"Current Position = " << pos << ", starting position = " << cs->GetPos() << ", size = " << cs->GetSize());			//kts 12/17/99 2:23PM
    // the 4 is the size of the IFF size field
	_out->seekp( cs->GetPos(), std::ios::beg );

    long size = cs->GetSize();
	assert( sizeof( long ) == 4 );
	_out->write( (char const*)&( size ), sizeof( long ) );
	_out->seekp( pos, std::ios::beg );

	if ( !chunkSize.empty() )
	{
		ChunkSizeBackpatch* csParent = chunkSize.back();
	    assert( cs );
		int remainder = cs->GetSize() % 4;
        if(cs->GetSize() > 0)
		    csParent->AddToSize(cs->GetSize());
        if(remainder ? 4 - remainder : 0)
		    csParent->AddToSize(remainder ? 4 - remainder : 0);
		csParent->AddToSize(4 + 4);		// ID, size
	}

	align( 4 );

	return cs;
}


IffWriterBinary::IffWriterBinary( std::ostream& out ) : _IffWriter()
{
	_out = &out;
	_out->setf( std::ios::showpoint );
}


IffWriterBinary::~IffWriterBinary()
{
	assert( _out );
}


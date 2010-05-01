//=============================================================================
// text.cc
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
#include <math/scalar.hp>
#include <iffwrite/iffwrite.hp>
#include <time.h>
#include <string>
#include <fstream>
#include <iomanip>

inline int round( const int n, const int align )
{
	return ( (n+align-1) / align) * align;
}


#if 0
void
IffWriterText::out_strstream( std::strstream& str )
{
	assert( _out );
	unsigned char* sz = (unsigned char*)str.str();

	int nChars = strlen( str.str() );	// not actually what I want
	for ( int i=0; i<nChars; ++i )
		*this << *sz++;
}
#endif

void
IffWriterText::out_int8( unsigned char i )
{
	assert( _out );
	*_out << int( i ) << "y ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
#endif
}


void
IffWriterText::out_int16( unsigned short i )
{
	assert( _out );

	*_out << int( i ) << "w ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
#endif
}


#if 0
void
IffWriterText::out_int32( unsigned long i )
{
	assert( _out );

	*_out << i << "l ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
#endif
}
#endif

void
IffWriterText::out_int32( int32 i )
{
	assert( _out );

	*_out << i << "l ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
#endif
}

void
IffWriterText::out_scalar( const Scalar& s )
{
	assert( _out );

	*_out << s;
	if ( Scalar( s.WholePart(), 0 ) == s )
		*_out << ".0";
	*_out << "(1,15,16) ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( int );
#endif
}



static char szSize[] = { '!', 'b', 'w', '!', 'l' };

void
IffWriterText::align( int cbAlign )
{
	assert( 0 <= cbAlign && cbAlign <= 4 );
}


void
IffWriterText::out_id( const ID& id )
{
	assert( _out );
	*_out << id << ' ';
}


#if 0
void
IffWriterText::out_string( const char* str )
{
	DBSTREAM1( ciff << "\"" << str << "\" "; )
	assert( _out );

	*_out << '"';
	// break lines at CR/LF, convert " to \"
	for ( char* pStr = (char*)str; *pStr; ++pStr )
	{
		if ( *pStr == '\n' )
			*_out << "\\n\"" << std::endl << '"';
#if 0
		else if ( *pStr == '\\' )
			*_out << "\\\\";
		else if ( *pStr == '"' )
			*_out << "\\\"";
#endif
		else
			*_out << *pStr;
	}
	*_out << "\" ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += strlen( str ) + 1;
#endif
}
#endif


void
IffWriterText::out_string( const std::string& str )
{
	DBSTREAM1( ciff  << "\"" << str.c_str() << "\" "; )
	assert( _out );

	*_out << '"';
	// break lines at CR/LF, convert " to \"
	for ( char* pStr = (char*)( str.c_str() ); *pStr; ++pStr )
	{
		if ( *pStr == '\n' )
			*_out << "\\n\"" << std::endl << '"';
		else if ( *pStr == '\\' )
			*_out << "\\\\";
		else if ( *pStr == '"' )
			*_out << "\\\"";
		else
			*_out << *pStr;
	}
	*_out << "\" ";
}


void
IffWriterText::out_string_continue( const std::string& str )
{
	out_string( str );
}


void
IffWriterText::out_mem( void* ptr, size_t size )
{
	assert( _out );

	char* pMem = (char*)ptr;
	char* pEnd = pMem + size;
	int nItemsOnLine = 0;
	while ( pMem < pEnd )
	{
		*_out << int( *pMem++ ) << "y ";
		++nItemsOnLine;
		if ( nItemsOnLine == 100 )
		{
			out_comment( Comment( "" ) );
			nItemsOnLine = 0;
		}
	}
	assert( pMem == pEnd );
	out_comment( Comment( "" ) );

	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->AddToSize(size);
}


#if 0
void
IffWriterText::out_int( long i )
{
	assert( _out );

	*_out << int( i ) << " ";

#if 0
	ChunkSizeBackpatch* cs = chunkSize.back();
	assert( cs );
	cs->size += sizeof( i );
#endif
}
#endif


void
IffWriterText::out_comment( const Comment& comment )
{
	*_out << " //" << comment() << std::endl << tab( chunkSize.size()+1 );
}


void
IffWriterText::enterChunk( const ID& id )
{
	assert( _out );

	*_out << std::endl << tab( chunkSize.size() ) << "{ ";

	_IffWriter::enterChunk( id );

	*this << id;
}


ChunkSizeBackpatch*
IffWriterText::exitChunk()
{
	assert( _out );

	ChunkSizeBackpatch* cs = _IffWriter::exitChunk();
	assert( cs );

	*_out << std::endl << tab( chunkSize.size() ) << "}";

	align( 4 );

	return cs;
}


IffWriterText::IffWriterText( std::ostream& out ) : _IffWriter()
{
	_out = &out;
	_out->setf( std::ios::showpoint );
	*_out << std::setprecision( 16 );
}


IffWriterText::~IffWriterText()
{
	assert( _out );
}

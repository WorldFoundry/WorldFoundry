//=============================================================================
// binstrm.cc:
// Copyright ( c ) 1996,97,98,99 World Foundry Group  
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
// Description:
// Original Author: Brad McKee
//============================================================================

#define _BINSTRM_CC

//=============================================================================

#include <streams/binstrm.hp>
#include <pigsys/endian.h>
#include <math/scalar.hp>
#include <cpplib/stdstrm.hp>
#include <hal/hal.h>

// ------------------------------------------------------------------------
// Debug Stream Support
// ------------------------------------------------------------------------

#if SW_DBSTREAM > 0
#if defined(_MSC_VER) || defined(__LINUX__)
#include <cpplib/strmnull.hp>
CREATENULLSTREAM(binstream_null);		 		// must be in same file to ensure proper order of construction
#define NULL_STREAM binstream_null
#endif
#endif

//==============================================================================

DBSTREAM1( CREATEANDASSIGNOSTREAM(cbinstrm,NULL_STREAM); )
DBSTREAM1( CREATEANDASSIGNOSTREAM(cstreaming,NULL_STREAM); )
DBSTREAM1( CREATEANDASSIGNOSTREAM(ctest,NULL_STREAM); )

// DBSTREAM1( ostream_withassign cbinstrm( NULL_STREAM); )
// DBSTREAM1( ostream_withassign cstreaming( NULL_STREAM ); )
// DBSTREAM1( ostream_withassign ctest( NULL_STREAM ); )

// ------------------------------------------------------------------------
// Validate compile-time switches
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// Binary I/O Stream Base Class
// ------------------------------------------------------------------------

binios::binios
(
	const iostate state,
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
	: _iostate( state ),
	_align8( align8 ), _align16( align16 ), _align32( align32 ), _alignobject( alignobject ),
	_word_order( order ), _scalar_type( sc_type )
{
	binios::Validate( __FILE__, __LINE__ );
}

// ------------------------------------------------------------------------

uint16
binios::process_uint16( const uint16 i ) const
{
	if( _word_order != binios::native_word_order )
		return utl_flipSW( i );
	else
		return i;
}

// ------------------------------------------------------------------------

int16
binios::process_int16( const int16 i ) const
{
	if( _word_order != binios::native_word_order )
		return utl_flipSW( i );
	else
		return i;
}

// ------------------------------------------------------------------------

uint32
binios::process_uint32( const uint32 i ) const
{
	if( _word_order != binios::native_word_order )
		return utl_flipLW( i );
	else
		return i;
}

// ------------------------------------------------------------------------

int32
binios::process_int32( const int32 i ) const
{
	if( _word_order != binios::native_word_order )
		return utl_flipLW( i );
	else
		return i;
}

// ------------------------------------------------------------------------
// Binary Input Stream Declaration
// ------------------------------------------------------------------------

// ------------------------------------------------------------------------
// given a file name, read entire file into memory

#if DO_DEBUG_FILE_SYSTEM			//!SW_STREAMING

binistream::binistream
(
	char const * name,
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
	:
	binios( binios::goodbit, align8, align16, align32, alignobject, order, sc_type ),
	_parent( NULL )
{
	DBSTREAM1( cbinstrm << "Binistream loaded " << name << " by name." << std::endl; )
	ConstructBinistreamFromFilename(name, align8, align16, align32, alignobject, order, sc_type);
}
#endif

// ------------------------------------------------------------------------
// given a memory buffer, construct a binistream to read from it

binistream::binistream
(
	const void* memory,
	int32 len,
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
	:
	binios( binios::goodbit, align8, align16, align32, alignobject, order, sc_type ),
	_parent( NULL )
{
	DBSTREAM1( cbinstrm << "Binistream created from  " << memory << " with a len of " << len << std::endl; )
	ConstructBinistreamFromMemory(memory,len, align8, align16, align32, alignobject, order, sc_type);
}


#if 0
// ------------------------------------------------------------------------
// Allocate a sub-stream within a parent stream.  The destructor updates parent.
// While a sub-stream is in existence, using the parent is illegal.

binistream::binistream
(
	binistream& parent,
	binios::streampos nbytes
)
	:
	binios
	(
		parent._iostate,
		parent.align8(),
		parent.align16(),
		parent.align32(),
		parent.alignobject(),
		parent.wordorder(),
		parent.scalartype()
	),
	_parent( &parent ),
	_buf( parent._cur ),
	_end( parent._cur + nbytes ),
	_cur( parent._cur )
{
	DBSTREAM2( cstreaming << "binistream::binistream( substream ) of nbytes " << nbytes << std::endl; )
	DBSTREAM2( ctest << "substream (" << nbytes << ")" << std::endl; )
	DBSTREAM2(cstreaming << "nbytes " << nbytes << " _parent->_cur + nbytes " << (void *)( _parent->_cur + nbytes) << " _parent->_end " << (void*)(_parent->_end)  << std::endl;)
	assert( _parent->_cur + nbytes <= _parent->_end );
	ValidateObject( *this );
}
#endif

#if 0		// kts 12-06-97 09:02am
!SW_STREAMING
// ------------------------------------------------------------------------
// Open a stream given a 32-bit PackedAssetID (generated by Levelcon)

binistream::binistream
(
	const packedAssetID assetID,
	const char* (*fp)(packedAssetID),			// function pointer to LookUpAssetName() function
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
	:
	_parent( NULL )
{
	// Look up assetID in Level asset list, and fetch the filename
	assert(fp);
	const char* name = (*fp)(assetID.ID());
	AssertMsg(name, "Binistream constructor couldn't resolve asset ID <" << assetID << '>');

	DBSTREAM3( cbinstrm << "binistream::binistream(): Found assetID <" << assetID <<
					">, matching <" << name << ">." << std::endl; )
	ConstructBinistreamFromFilename(name, align8, align16, align32, alignobject, order, sc_type);
}
#endif

// ------------------------------------------------------------------------

binistream::~binistream( void )
{
	ValidateObject( *this );
	DBSTREAM2( cbinstrm << "binistream::~binistream()" << std::endl; )
	DBSTREAM2( cstreaming << "binistream::~binistream()" << std::endl; )
	if( _parent )
	{
		// make sure that we've read to end of substream
		assert( _cur == _end ); // seekg( binios::streampos( _end - _cur ), binios::cur );
		if( _parent->_cur == _parent->_end )
			_parent->_iostate |= binios::eofbit;
		else if( _parent->_cur > _parent->_end )
			_parent->_iostate |= ( binios::eofbit | binios::badbit );
		_parent->_cur		= _cur;
	}
	else // owns the _buf array
	{
		if( _buf && _ownMemory)
			HALLmalloc.Free((void*)_buf);
			//DELETE_ARRAY( (char*)_buf );
	}
	_buf = NULL;
	_end = NULL;
	_cur = NULL;
	_iostate = binios::badbit;
}

// ------------------------------------------------------------------------

const binistream&
binistream::operator = (const binistream& other)
{
	assert( !other._ownMemory );	// cannot copy a binistream if we own the memory
#pragma message ("KTS: do this right")
	memcpy(this,&other,sizeof(binistream));  // kts this is not the way to do this
	return *this;
}

// ------------------------------------------------------------------------

void
binistream::ConstructBinistreamFromMemory
(
	const void* memory,
	int32 len,
	const alignment,
	const alignment,
	const alignment,
	const alignment,
	const word_order,
	const scalar_type
)
{
	ValidateObject( *this );

	assert(ValidPtr(memory));
	assert(len > 0);
	assert(len < 512000);				// kts arbitrary large number

	// set up end and current pointers
	_buf = (const char*)memory;
	_end = _buf + len;
	_cur = _buf;
	_ownMemory = false;
}

// ------------------------------------------------------------------------

#if DO_DEBUG_FILE_SYSTEM			//!SW_STREAMING

#include <pigsys/genfh.hp>

void
binistream::ConstructBinistreamFromFilename
(
	const char* name,
	const alignment,
	const alignment,
	const alignment,
	const alignment,
	const word_order,
	const scalar_type
)
{
	ValidateObject( *this );
	DBSTREAM2( cbinstrm << "binistream::binistream( " << name << " )" << std::endl; )
	DBSTREAM2( cstreaming << "binistream::binistream( " << name << " )" << std::endl; )

//#if defined( __PSX__ ) && DO_ASSERTIONS
//	{
//	char szFilename[_MAX_FNAME];
//	char szExt[_MAX_FNAME];
//	SplitPath( name, NULL, NULL, szFilename, szExt );
//	AssertMsg( strlen( szFilename ) <= 8, "Filename \"" << szFilename << "\" is too long!" );
//	AssertMsg( strlen( szExt ) <= 4, "Filename extension \"" << szExt << "\" is too long!" );
//	}
//#endif

	// open file
	int fh = FHOPENRD( name );
	if( fh != -1 )
	{
		DBSTREAM3( cprogress << "Binistream loading " << name << std::endl; )
		// determine file length
		int seekok = FHSEEKEND( fh, 0 );
		AssertMsg( seekok != -1, "Error during seek" );
		int32 len = FHTELL( fh );
		seekok = FHSEEKABS( fh, 0 );
		AssertMsg( seekok != -1, "Error during seek" );

		// allocate a buffer, read in data
		if( len > 0 )
		{
			//char* memory = NEW_ARRAY( char[len] );
			char* memory = new (HALLmalloc) char[len];
			ValidatePtr( memory );
			_ownMemory = true;
			::FHREAD( fh, memory, len );
			_buf = memory;
		}
		else _buf = NULL;

		// close file
		FHCLOSE( fh );

		// set up end and current pointers
		_end = _buf + len;
		_cur = _buf;
	}
	else
	{
		_iostate = binios::badbit;
		DBSTREAM1( cdebug << "binistream::binistream(): Cannot open file " << name << std::endl; )
		AssertMsg(0, "binistream::binistream: unable to open file " << name);
		printf("binistream::binistream: unable to open file %s", name);
		exit(1);
	}
}

#endif

// ------------------------------------------------------------------------

binistream&
binistream::operator >> ( binistream& (*f)( binistream& ) )
{
	return f( *this );
}

// ------------------------------------------------------------------------
// Binary I/O Stream (was Inline Functions)
// ------------------------------------------------------------------------

#if 0

binistream&
binistream::read( char * buf, int len )
{
        DBSTREAM4( cbinstrm << "binistream::read( " << ((void *)buf) << ", " << len << " )"
                << std::endl; )
        char * next_cur = _cur + len;
        DBSTREAM4( ctest << "binistream::read( " << len << " ) to " << (void *)next_cur << std::endl; )

		if( next_cur < _end )
        {
                while( _cur < next_cur ) *buf++ = *_cur++;
        }
        else if( next_cur == _end )
        {
                while( _cur < next_cur ) *buf++ = *_cur++;
                _iostate |= binios::eofbit;
        }
        else
        {
                _iostate |= ( binios::failbit | binios::eofbit );
                assert( 0 );
        }
        return *this;
}

#endif

// ------------------------------------------------------------------------

binistream&
binistream::seekg( binios::streamoff offset, binios::seekdir direction )
{
        DBSTREAM4( cbinstrm << "binistream::seekg( " << offset << ", "; )
        DBSTREAM2( ctest << "binistream::seekg( " << offset << ", "; )

        const char * newcur;

        switch( direction )
        {
                case binios::beg:               // seek from beginning
                        newcur = _buf + offset;
                        DBSTREAM2( ctest << "binios::beg ) to " << (void *)newcur << std::endl; )
                break;

                case binios::cur:               // seek from current pos
                        newcur = _cur + offset;
                        DBSTREAM2( ctest << "binios::cur ) to " << (void *)newcur << std::endl; )
                break;

                case binios::end:               // seek from end of stream
                        newcur = _end - offset;
                        DBSTREAM2( ctest << "binios::end ) to " << (void *)newcur << std::endl; )
                break;

                default:						// nothing else allowed
						newcur=NULL;
                        Fail( "Illegal binistream::seekdir" );
        }

        if( newcur >= _buf && newcur <= _end )  // if within range
                _cur = newcur;                  // set new pos
        else
        {
                _iostate |= binios::failbit; 	// else set fail flag
                assert( 0 );
        }
        return *this;
}

// ------------------------------------------------------------------------

binistream&
binistream::readheader( const uint32 blocktype, const uint32 blocksize )
{
	// set up word order
	// if blocktype is correct ("pf3d"), then stream is in native order.
	if( blocktype == ( ( 'p' << 24 ) | ( 'f' << 16 ) | ( '3' << 8 ) | 'd' ) )
	{
		_word_order = binios::native_word_order;
	}
	// else if it's reversed ("d3fp"), then it's in the non-native order
	else if( blocktype == ( ( 'd' << 24 ) | ( '3' << 16 ) | ( 'f' << 8 ) | 'p' ) )
	{
		// if native is little-, stream must be big-endian
		// else it must be little-endian.
		_word_order = ( binios::native_word_order == binios::little_endian )
			? binios::big_endian : binios:: little_endian;
	}
	// else it's not recognized.
	else
		Fail( "Unknown header blocktype" );

  #if SW_STREAM_STRICT_FORMATTING
	if( _word_order != binios::native_word_order )
	{
		cerror << "The endian-ness of this stream does not match this platform";
		sys_exit( -1 );
	}
  #endif // SW_STREAM_STRICT_FORMATTING

	// make sure the amount of data is correct
	(void)blocksize;
	AssertMsg( blocksize == 5 * sizeof( uint32 ),
		"This stream does not contain enough data to hold a pfm3d header block" );

	DBSTREAM1(
	// check the version
	uint32 version;
	*this >> version;
	if( version < binios::pfm3d_version )
	{
		cerror << "Expected version id " << binios::pfm3d_version << " but found " << version << std::endl;
		Fail( "Unable to read an old pfm3d file...try rebuilding the pf3 files" );
	}
	else if( version > binios::pfm3d_version )
	{
		cerror << "Expected version id " << binios::pfm3d_version << " but found " << version << std::endl;
		Fail( "Unable to read a pfm3d that is more current than the source...try rebuild the source" );
	}
	)

	// set the scalar type and alignments
	*this
		>> _scalar_type
		>> _align8
		>> _align16
		>> _align32;

  #if SW_STREAM_STRICT_FORMATTING
	if( _scalar_type != binios::native_scalar_type )
	{
		cerror << "The scalar type of this stream does not match this platform";
		sys_exit( -1 );
	}
  #endif // SW_STREAM_STRICT_FORMATTING

	// make sure that everything is kosher
	ValidateObject( *this );

	return *this;
}

// ------------------------------------------------------------------------
// Binary Output Stream Declaration
// ------------------------------------------------------------------------

#if defined( WRITER )

#include <cpplib/genfh.hp>

binostream::binostream
(
	const char * name,
	const size_t bufsz,
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
	: binios( binios::goodbit, align8, align16, align32, alignobject, order, sc_type )
{
	DBSTREAM2( cbinstrm << "binostream::binostream( " << name << ", " << bufsz << " )" << std::endl; )

	_fh = FHOPENWR( name );
	if( _fh == -1 )
	{
		_buf = NULL;
		_end = NULL;
		_cur = NULL;
		_iostate |= ( binios::badbit | binios::failbit );
		assert ( 0 );	// LDH
		_pos = -1;
	}
	else
	{
		_buf = NEW_ARRAY( char[bufsz] );
		ValidatePtr( _buf );
		_end = _buf + bufsz;
		_cur = _buf;
		_iostate = binios::goodbit;
		_pos = 0;
	}
	ValidateObject( *this );
}

// ------------------------------------------------------------------------

binostream::~binostream( void )
{
	ValidateObject( *this );
	DBSTREAM2( cbinstrm << "binostream::~binostream()" << std::endl; )

	if( _fh != -1 )
	{
		flush();
		FHCLOSE( _fh );
	}

	if( _buf )
	{
		DELETE_ARRAY( _buf );
		_buf = NULL;
	}
	_cur = NULL;
	_end = NULL;
	_pos = -1;
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const char c )
{
	DBSTREAM4( cbinstrm << "binostream::operator << char( " << c << " )" << std::endl; )
	align( _align8 );
	return write( &c, sizeof( c ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const uint8 i )
{
	DBSTREAM4( cbinstrm << "binostream::operator << uint8( " << i << " )" << std::endl; )
	align( _align8 );
	return write( (const char *)&i, sizeof( i ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const int8 i )
{
	DBSTREAM4( cbinstrm << "binostream::operator << int8( " << i <<	" )" << std::endl; )
	align( _align8 );
	return write( (const char *)&i, sizeof( i ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const uint16 i )
{
	DBSTREAM4( cbinstrm << "binostream::operator << uint16( " << i << " )" << std::endl; )
	align( _align16 );
	uint16 flipped = process_uint16( i );
	return write( (const char *)&flipped, sizeof( flipped ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const int16 i )
{
	DBSTREAM4( cbinstrm << "binostream::operator << int16( " << i << " )" << std::endl; )
	align( _align16 );
	int16 flipped = process_int16( i );
	return write( (const char *)&flipped, sizeof( flipped ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const uint32 i )
{
	DBSTREAM4( cbinstrm << "binostream::operator << uint32( " << i << " )" << std::endl; )
	align( _align32 );
	uint32 flipped = process_uint32( i );
	return write( (const char *)&flipped, sizeof( flipped ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( const int32 i )
{
	DBSTREAM4( cbinstrm << "binostream::operator << int32( " << i << " )" << std::endl; )
	align( _align32 );
	int32 flipped = process_int32( i );
	return write( (const char *)&flipped, sizeof( flipped ) );
}

// ------------------------------------------------------------------------

binostream&
binostream::operator << ( binostream& (*f)( binostream& ) )
{
	return f( *this );
}

// ------------------------------------------------------------------------

binostream&
binostream::write( char const * buf, int len )
{
	DBSTREAM4( cbinstrm << "write( " << (void *)buf << ", " << len << " )" << std::endl; )

	AssertMsg( good(), "Can't write to bad binostream" );

	if( _cur + len < _end )
	{
		memcpy( _cur, buf, len );
		_cur += len;
	}
	else // can't fit in buffer, flush and try again
	{
		flush();
		if( _cur + len >= _end )
		{
			int nwrote = ::FHWRITE( _fh, buf, len );
			if( nwrote != len )
			{
				_iostate |= binios::failbit;
				assert( 0 );	// LDH
			}
		}
		else
		{
			memcpy( _cur, buf, len );
			_cur += len;
		}
	}

	_pos += len;
	return *this;
}

// ------------------------------------------------------------------------

binostream&
binostream::writeheader( void )
{
	uint32 blocktype = ( 'p' << 24 ) | ( 'f' << 16 ) | ( '3' << 8 ) | 'd';

	*this << blocktype;
	// make sure to update this if fields are added
	*this << uint32( sizeof( uint32 ) * 5 );

	return *this << uint32( binios::pfm3d_version )
		<< _scalar_type
		<< _align8
		<< _align16
		<< _align32;
}

// ------------------------------------------------------------------------

binostream&
binostream::flush( void )
{
	DBSTREAM4( cbinstrm << "flush()" << std::endl; )

	AssertMsg( good(), "Can't flush a bad binostream" );

	if( _cur - _buf > 0 )
		::FHWRITE( _fh, _buf, _cur - _buf );
	_cur = _buf;

	return *this;
}

// ------------------------------------------------------------------------

binostream&
binostream::seekp( streampos position )
{
	return seekp( position, binios::beg );
}

// ------------------------------------------------------------------------

binostream&
binostream::seekp( streamoff offset, seekdir direction )
{
	AssertMsg( good(), "Can't seek a bad binostream" );

	flush();

	DBSTREAM4( cbinstrm << "seekp( " << offset << ", "; )

	int result;
	switch( direction )
	{
		case binios::beg:
			DBSTREAM4( cbinstrm << "binios::beg )" << std::endl; )
			result = FHSEEKABS( _fh, offset );
		break;

		case binios::cur:
			DBSTREAM4( cbinstrm << "binios::cur )" << std::endl; )
			result = FHSEEKREL( _fh, offset );
		break;

		case binios::end:
			DBSTREAM4( cbinstrm << "binios::end )" << std::endl; )
			result = FHSEEKEND( _fh, offset );
		break;

		default:
			Fail( "Illegal binios::seekdir" );
	}
	if( result == -1 )
	{
		_iostate |= binios::failbit;
		assert( 0 );	// LDH
	}

	else _pos = result;
	return *this;
}

// ------------------------------------------------------------------------

binios::streampos
binostream::tellp( void ) const
{
	AssertMsg( good(), "Can't tellp a bad binostream" );

	DBSTREAM4( cbinstrm << "tellp() = " << _pos << std::endl; )

	return _pos;
}

// ------------------------------------------------------------------------
// pow2_boundary = 2, 4, 8, 16, 32, etc.
// returns true if stream position is on a boundary

int
binostream::aligned( const size_t pow2_boundary ) const
{
	return tellp() & ( pow2_boundary - 1 ) == 0;
}

// ------------------------------------------------------------------------
// makes sure that tellg % pow2_boundary - 1 == 0, seeks if necessary

binostream&
binostream::align( const int pow2_boundary )
{
	assert( pow2_boundary <= 4 );
	AssertMsg( good(), "Can't align a bad binostream" );

	DBSTREAM4( cbinstrm << "align( " << pow2_boundary << " )" << std::endl; )

	binios::streamoff extra = tellp() & ( pow2_boundary - 1 );
	if( extra != 0 ) seekp( 4 - extra, binios::cur );
	AssertMsg( extra >= 0 && extra <= pow2_boundary - 1,
		"Alignment pad length is out of range!" );
	return *this;
}

#endif	// WRITER
//=============================================================================

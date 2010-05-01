//=============================================================================
// sgin.cc (savegame input)
// by William B. Norris IV
// Copyright ( c ) 1997,99 World Foundry Group  
//=============================================================================
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

//=============================================================================
// Interface to load saved game files

#include <savegame/savegame.hp>
#include <pigsys/minmax.hp>

extern char szGameName[];

//=============================================================================

saveistream::saveistream
(
	char const* name,
	const size_t bufsz,
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
{
	_bufsz = bufsz;
#if defined( __PSX__ )
	AssertMsg( bufsz == 128, "Memory card can only read in 128 byte chunks" );
#endif
	_buf = (const char*)malloc( bufsz );
	assert( _buf );
	AssertMemoryAllocation(_buf);
//	_cur = (char*)_buf;		pagein() does this
	_end = _buf + bufsz;

	// checksum is on 32-bit entries so _bufsz must be a multiple of 32
	assert( ( _bufsz % sizeof( int32 ) ) == 0 );

	// TODO: don't like saving a copy of this, but checksum() uses it to re-open the file
	_name = strdup( name );
	assert( _name );

	_fh = openSaveFile( _name );
	assert( _fh != -1 );

	pagein();
	readHeader();			// just to get _filesize

	reset();

	int32 _checksum = checksum();
	// TODO: turn into error message
	assert( _checksum == 0 );

	pagein();
	readHeader();
}

// ------------------------------------------------------------------------

saveistream::~saveistream()
{
	assert( _name );
	free( (void*)_name );

	closeSaveFile();
}

// ------------------------------------------------------------------------

void
saveistream::reset()
{
	// reset to beginning
	closeSaveFile();
	_fh = openSaveFile( _name );
	assert( _fh != -1 );
}

// ------------------------------------------------------------------------

int32
saveistream::checksum()
{
	int32 _checksum = 0;
	assert( _filesize > 0 );
	assert( _bufsz > 0 );

	for ( int i=0; i<_filesize; i+=_bufsz )
	{
		if ( pagein() )
		{
			int32* pChecksum = (int32*)_buf;
			assert( ( _bufsz % sizeof( int32 ) ) == 0 );
			for ( int l=0; l<_bufsz / sizeof( int32 ); ++l, ++pChecksum )
			{
				_checksum ^= *pChecksum;
			}
		}
	}

	reset();

	return _checksum;
}

// ------------------------------------------------------------------------

bool
saveistream::pagein()
{
	int32 cbRead = readSaveFile();
	assert( ( cbRead == 0 ) || ( cbRead == _bufsz ) );

	_cur = (char*)_buf;

	return bool( cbRead );
}

// ------------------------------------------------------------------------

saveistream&
saveistream::align( const int pow2_boundary )
{
    assert( pow2_boundary <= 4 );

    AssertMsg( good(), "Can't align a bad binistream" );

    DBSTREAM4( cbinstrm << "align( " << pow2_boundary << " )" << std::endl; )

    binios::streamoff extra = tellg() & ( pow2_boundary - 1 );

    assert( 0 <= extra && extra < 4 );

	int32 _bitBucket;
    if ( extra != 0 )
		read( (char*)&_bitBucket, 4 - extra );

    return *this;
}

// ------------------------------------------------------------------------

saveistream::streampos
saveistream::tellg() const
{
	assert( good() );
	assert( _cur );
	assert( _buf );

	return streampos( _cur - _buf );
}

// ------------------------------------------------------------------------

saveistream&
saveistream::read( char * buf, int len )
{
    DBSTREAM4( cbinstrm << "binistream::read( " << ((void *)buf) << ", " << len << " )" << std::endl; )

//	cout << "_cur = " << (void*)_cur << " offset = " << hex << _cur - _buf << std::endl;

	if ( _cur + len <= _end )
	{
		memcpy( buf, _cur, len );
		_cur += len;
		if ( _cur == _end )
			pagein();
	}
	else
	{
		char* brokenBuf = (char*)buf;
		// Write however many bytes are left in buffer (this will flush the buffer)
		int cbLeft = _end - _cur;
		read( brokenBuf, cbLeft );
		brokenBuf += cbLeft;

		// Write out the remainder in _bufsz chunks
		for ( cbLeft = len - cbLeft ; cbLeft > 0; brokenBuf += _bufsz, cbLeft -= _bufsz )
			read( brokenBuf, MIN( cbLeft, _bufsz ) );
	}

	return *this;
}

// ------------------------------------------------------------------------

saveistream&
saveistream::readHeader()
{
	char szHeader[ 512 ];
	*this >> szHeader;
	*this >> _filesize;
	assert( ( _filesize % _bufsz ) == 0 );

	DBSTREAM1( std::cout << "Header = \"" << szHeader << "\" of " << _filesize << " bytes maximum" << std::endl; )

	// TODO: Can't do an assertion here.  Need to have some error code.
	assert( strcmp( szGameName, szHeader ) == 0 );

	return *this;
}

// ------------------------------------------------------------------------

// TODO: Add maximum count?
saveistream&
saveistream::operator >>( char* str )
{
	char* pch = str;
	do
	{
		*this >> *pch++;
	} while ( *( pch-1 ) );

	return *this;
}

// ------------------------------------------------------------------------

saveistream&
saveistream::operator >> ( char& c )
{
    DBSTREAM4( cbinstrm << "saveistream::operator >> ( char ) begin" << std::endl; )

    align( _align8 );
    read( &c, sizeof( c ) );

    DBSTREAM4( cbinstrm << "saveistream::operator >> ( char ) = " << uint32( c ) << std::endl; )

    return *this;
}

// ------------------------------------------------------------------------

saveistream&
saveistream::operator >> ( uint32& i )
{
    DBSTREAM4( cbinstrm << "saveistream::operator >> ( uint32 ) begin" << std::endl; )

    align( _align32 );
    read( (char *)&i, sizeof( i ) );
#if !SW_STREAM_STRICT_FORMATTING
    i = process_uint32( i );
#endif // !SW_STREAM_STRICT_FORMATTING

    DBSTREAM4( cbinstrm << "saveistream::operator >> ( uint32 ) = " << i << std::endl; )

    return *this;
}

// ------------------------------------------------------------------------

saveistream&
saveistream::operator >> ( int32& i )
{
    DBSTREAM4( cbinstrm << "saveistream::operator >> ( int32 ) begin" << std::endl; )

    align( _align32 );
    read( (char *)&i, sizeof( i ) );
#if !SW_STREAM_STRICT_FORMATTING
    i = process_int32( i );
#endif /* !SW_STREAM_STRICT_FORMATTING */

    DBSTREAM4( cbinstrm << "saveistream::operator >> ( int32 ) = " << i << std::endl; )

    return *this;
}

// ------------------------------------------------------------------------

saveistream&
saveistream::operator >>( Scalar& scalar )
{
    DBSTREAM4( cbinstrm << "saveistream::operator >> ( QScclar ) begin" << std::endl; )
	align( _align32 );
	read( (char*)&scalar, sizeof( scalar ) );

    DBSTREAM4( cbinstrm << "saveistream::operator >> ( Scalar ) = " << scalar << std::endl; )

	return *this;
}

// ------------------------------------------------------------------------

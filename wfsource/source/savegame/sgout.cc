// sgout.cc (savegame output)

// Interface to save game files
//
// by William B. Norris IV
// Copyright 1997,99 World Foundry Group.  

#include <savegame/savegame.hp>
#include <pigsys/minmax.hp>

saveostream::saveostream
(
	char const * name,
	const size_t filesize,
	const size_t bufsz,
	const alignment align8,
	const alignment align16,
	const alignment align32,
	const alignment alignobject,
	const word_order order,
	const scalar_type sc_type
)
{
	// create a common function to do the buffer management and
	// move _nBlocks et al to PSX version of constructor
	_bufsz = bufsz;
	_filesize = ROUND( filesize, _bufsz );
	_filesizeRounded = ROUND( _filesize, 8192 );
	assert( ( _filesizeRounded % 8192 ) == 0 );
	_nBlocks = _filesizeRounded / 8192;
#if defined( __PSX__ )
	AssertMsg( bufsz == 128, "Memory card can only write in 128 byte chunks" );
#endif
	_buf = (const char*)malloc( bufsz );
	assert( _buf );
	AssertMemoryAllocation(_buf);
	_cur = (char*)_buf;
	_end = _buf + bufsz;

	// checksum is on 32-bit entries so _bufsz must be a multiple of 32
	assert( ( _bufsz % sizeof( int32 ) ) == 0 );
	_checksum = 0;

	_cbWritten = 0;
	_fh = createSaveFile( name );
	assert( _fh != -1 );
}

// ------------------------------------------------------------------------

saveostream::~saveostream()
{
	align( _align32 );
	*this << checksum( tellp() );
	// Write out the remainder of the block as 0's so the checksum works out
	char _zero = 0;
	while ( _cur != _buf )
		*this << _zero;
	// flush should have happended at end of zeros
	assert( _checksum == 0 );
	assert( _cur == _buf );

	DBSTREAM1(
		std::cout << "Wrote a total of " << _cbWritten
			<< " bytes to saved game out of a total of " << _filesize
			<< " bytes available" << std::endl;
		std::cout << "Checksum = " << std::hex << _checksum << std::dec << std::endl;
	)
	assert( _cbWritten <= _filesizeRounded );	// way overshot
	assert( _cbWritten <= _filesize );			// just overshot specified size

	assert( _buf );
	free( (char*)_buf );

	closeSaveFile();
}

// ------------------------------------------------------------------------

extern char szGameName[];

saveostream&
saveostream::writeHeader()
{
	*this << szGameName;
	*this << _filesize;

	return *this;
}
// ------------------------------------------------------------------------

int32
saveostream::checksum( size_t cb )
{
	int32 CHECKSUM = _checksum;

	int32* pChecksum = (int32*)_buf;
	assert( ( cb % sizeof( int32 ) ) == 0 );
	for ( int i=0; i<cb/sizeof( int32 ); ++i, ++pChecksum )
	{
		CHECKSUM ^= *pChecksum;
		DBSTREAM4( cout << "CHECKSUM = " << hex << CHECKSUM << " (" << *pChecksum << ')' << dec << std::endl; )
	}

	return CHECKSUM;
}

// ------------------------------------------------------------------------

saveostream&
saveostream::flush()
{
	// Add to checksum
	_checksum = checksum( _bufsz );

	assert( _fh != -1 );
	// always write _bufsz bytes (PlayStation can only write in 128 byte chunks)
	unsigned int cbWritten = writeSaveFile();
//?	if ( cbWritten != _bufsz )
//?		_iostate |= binios::failbit;
	assert( cbWritten == _bufsz );
	_cur = (char*)_buf;

	return *this;
}

// ------------------------------------------------------------------------

saveostream&
saveostream::align( const int pow2_boundary )
{
	static uint32 _zero = 0UL;

	assert( pow2_boundary <= 4 );
	AssertMsg( good(), "Can't align a bad saveostream" );

	DBSTREAM4( cbinstrm << "align( " << pow2_boundary << " )" << std::endl; )
	binios::streamoff extra = tellp() & ( pow2_boundary - 1 );
	if ( extra != 0 )
		write( (const char*)&_zero, 4 - extra );
	AssertMsg( extra >= 0 && extra <= pow2_boundary - 1, "Alignment pad length is out of range!" );

	return *this;
}

// ------------------------------------------------------------------------

saveostream&
saveostream::operator << ( const char c )
{
	DBSTREAM4( cbinstrm << "saveostream::operator << char( " << c << " )" << std::endl; )
	align( _align8 );

	return write( &c, sizeof( c ) );
}

// ------------------------------------------------------------------------

saveostream&
saveostream::operator << ( const uint32 i )
{
	DBSTREAM4( cbinstrm << "saveostream::operator << uint32( " << i << " )" << std::endl; )
	align( _align32 );
	uint32 flipped = process_uint32( i );

	return write( (const char *)&flipped, sizeof( flipped ) );
}

// ------------------------------------------------------------------------

saveostream&
saveostream::operator << ( const int32 i )
{
	DBSTREAM4( cbinstrm << "saveostream::operator << int32( " << i << " )" << std::endl; )
	align( _align32 );
	int32 flipped = process_int32( i );

	return write( (const char *)&flipped, sizeof( flipped ) );
}

// ------------------------------------------------------------------------

saveostream&
saveostream::operator << ( const Scalar scalar )
{
	DBSTREAM4( cbinstrm << "saveostream::operator << const Scalar( " << scalar << " )" << std::endl; )
	align( _align32 );
#if defined(SCALAR_TYPE_FIXED)
	int32 brs = scalar.AsLong();
	return write( (const char*)&brs, sizeof( int32 ) );
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   assert(0);        // kts floating point exporter needed
   return *this;
#else
#error SCALAR TYPE not defined
#endif
}

// ------------------------------------------------------------------------

saveostream&
saveostream::operator << ( const char* str )
{
	DBSTREAM4( cbinstrm << "saveostream::operator << const char*( " << str << " )" << std::endl; )
	//? align( _align8 );

	return write( str, strlen( str ) + 1 );
}

// ------------------------------------------------------------------------

saveostream&
saveostream::write( char const * buf, int len )
{
	DBSTREAM4( cbinstrm << "write( " << (void *)buf << ", " << len << " )"
		<< std::endl; )

	AssertMsg( good(), "Can't write to bad saveostream" );

	// Although this could be written more compactly using just this while loop,
	// I have "special cased" the simple case (and most likely) so that it doesn't
	// recurse into write() an additional time.
	if ( _cur + len <= _end )
	{
		memcpy( _cur, buf, len );
		_cur += len;
		_cbWritten += len;
		if ( _cur == _end )
			flush();
	}
	else
	{
		char* brokenBuf = (char*)buf;
		// Write however many bytes are left in buffer (this will flush the buffer)
		unsigned int cbLeft = _end - _cur;
		write( brokenBuf, cbLeft );
		brokenBuf += cbLeft;

		// Write out the remainder in _bufsz chunks
		for ( cbLeft = len - cbLeft ; cbLeft > 0; brokenBuf += _bufsz, cbLeft -= _bufsz )
			write( brokenBuf, MIN( cbLeft, _bufsz ) );
	}

	return *this;
}


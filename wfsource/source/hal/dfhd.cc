//=============================================================================
// DiskFileHD.cc:
// Copyright ( c ) 1996,97,99 World Foundry Group  
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
// Original Author: Kevin T. Seghetti
//============================================================================
// kts shouldn't this be in each platforms sub-directory?
//=============================================================================

#include <pigsys/pigsys.hp>
//#include <cpplib/stdstrm.hp>
//#include <streams/dbstrm.hp>
#include <pigsys/genfh.hp>
#include <hal/diskfile.hp>

extern void InitSimpleDisplay();
extern void UpdateSimpleDisplay();

//=============================================================================

#define DO_HDUMP 0

//=============================================================================

void
HalInitFileSubsystem()
{
#if defined(__PSX__)
#if DO_HDUMP
	InitSimpleDisplay();
#endif
	ASSERTIONS(int PCinit_err = ) PCinit();			// xina - initialize filesystem
	assert( PCinit_err == 0 );
#if DO_HDUMP
	FntPrint( "CDINIT" );
	UpdateSimpleDisplay();
#endif
#endif
}

//=============================================================================

void
DiskFileHD::Validate() const
{
	_DiskFile::Validate();
	assert(_fileHandle);
}

//=============================================================================


DiskFileHD::DiskFileHD( const char* fileName ) : _DiskFile( fileName )
{
	_currentFilePosition = 0;
	_hasSeeked = false;

#if defined( __WIN__ )
	// first try finding the data at the end of this executable
	extern char* * __argv;
	if ( _fileHandle = FHOPENRD( __argv[ 0 ] ) )
	{
		static const char szWorldFoundryTag[] = "WorldFoundry";

		int cbToRead = strlen( szWorldFoundryTag ) + sizeof( DWORD );
#if !DO_ASSERTIONS
		int32
#endif
		_fileSize = FHSEEKEND( _fileHandle, -cbToRead );
		if ( _fileSize != -1 )
		{
			AssertMsg( _fileSize != -1, "Error during seek" );

			char buffer[ 256 ];
			assert( cbToRead <= sizeof( buffer ) );
			int readok = FHREAD( _fileHandle, buffer, cbToRead );
			assert( readok == cbToRead );
			if ( strncmp( buffer+4, szWorldFoundryTag, strlen( szWorldFoundryTag ) ) == 0 )
			{
				DWORD cbData = *((DWORD*)buffer);
				_cbFileOffset = _fileSize - cbData;
				_fileSize = cbData;
				printf( "Found data in executable\n" );
				return;
			}
		}
	}
#endif

	// open file
	_fileHandle = FHOPENRD( fileName );
	AssertMsg( _fileHandle != -1, "filename = " << fileName );
	if ( _fileHandle != -1 )
	{
		// determine file length
		int seekok = FHSEEKEND( _fileHandle, 0 );
		AssertMsg( seekok != -1, "Error during seek" );
#if DO_ASSERTIONS
		_fileSize = FHTELL( _fileHandle );
#endif
		seekok = FHSEEKABS( _fileHandle, 0 );
		AssertMsg( seekok != -1, "Error during seek" );

		assert(_fileSize > 0);
//		DBSTREAM1( cprogress << "DiskFile opening " << fileName << " with a length of " << _fileSize << endl; )

		_cbFileOffset = 0;
	}
}

//=============================================================================

DiskFileHD::~DiskFileHD()
{
	Validate();
	// close file
	FHCLOSE( _fileHandle );
}

//=============================================================================

void
DiskFileHD::SeekRandom( int32 position )
{
#if DO_HDUMP
#pragma message( "remove" )
	InitSimpleDisplay();
	char szMsg[ 80 ];
	sprintf( szMsg, "SeekRandom: pos=%ld sector=%d\n", position, position / DiskFileCD::_SECTOR_SIZE );
	FntPrint( szMsg );
	UpdateSimpleDisplay();
#endif
	Validate();
	AssertMsg(position >= 0,"position requested = " << position);
	assert(position <= _fileSize);
	AssertMsg( position % DiskFileCD::_SECTOR_SIZE == 0, "position = " << position );

#if DO_ASSERTIONS
	int seekok =
#endif
		FHSEEKABS( _fileHandle, _cbFileOffset + position );
	AssertMsg( seekok != -1, "Error during seek" );

	_currentFilePosition = position;
	_hasSeeked = true;
}

//=============================================================================

void
HDump(char* title, void* buffer, ulong bufferSize);

void
DiskFileHD::ReadBytes( void* buffer, int32 size )
{
	assert(_hasSeeked == true);
	if(!_hasSeeked)
		FatalError(__FILE__ ": ReadBytes called without a seek");

#if DO_HDUMP
#pragma message( "remove" )
	InitSimpleDisplay();
	char szMsg[ 80 ];
	sprintf( szMsg, "ReadBytes: size=%ld, buffer = %p\n", size, buffer );
	FntPrint( szMsg );
	UpdateSimpleDisplay();
#endif
	Validate();
	assert(size > 0);
	assert(ValidPtr(buffer));
	AssertMsg( (_fileSize - _currentFilePosition) >= size, "_fileSize=" << _fileSize << ", _currentFilePosition=" << _currentFilePosition << ", size=" << size );
	assert(_currentFilePosition % DiskFileCD::_SECTOR_SIZE == 0);
	AssertMsg(size % DiskFileCD::_SECTOR_SIZE == 0, "size = " << size);

#if DO_ASSERTIONS
	int cbRead =
#endif
		::FHREAD( _fileHandle, buffer, size );
	assert( cbRead == size );
	_currentFilePosition += size;
	_hasSeeked = false;

#if DO_HDUMP
	HDump("Sector Dump:",((char*)buffer)-36, size);
#endif
}

//=============================================================================

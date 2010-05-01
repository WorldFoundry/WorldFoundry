//=============================================================================
// DiskFileCD.cc:
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

//=============================================================================

#include <pigsys/pigsys.hp>
#include <cpplib/stdstrm.hp>
#include <streams/dbstrm.hp>
#include <streams/diskfile.hp>

#include <windows.h>

//=============================================================================

void
DiskFileCD::Validate() const
{
	_DiskFile::Validate();
}

//=============================================================================

DiskFileCD::DiskFileCD( const char* filename ) : _DiskFile( filename )
{
	assert( ValidPtr( filename ) );
	assert( strlen( filename ) < _MAX_PATH );

	char cdFilename[ _MAX_PATH ];
	strcpy( cdFilename, "w:\\" );
	strcat( cdFilename, filename );

	mmio = mmioOpen( cdFilename, NULL, MMIO_READ );
	assert( mmio );

	int error = mmioSetBuffer( mmio, _buffer, 0, 0 );
	assert( error == 0 );

	_fileSize = mmioSeek( mmio, 0, SEEK_END );
	DBSTREAM1( cout << cdFilename << " is " << _fileSize << " bytes long" << endl; )
	mmioSeek( mmio, 0, SEEK_SET );
	assert( _fileSize > 0 );

	_currentFilePosition = 0;
}

//=============================================================================

DiskFileCD::~DiskFileCD()
{
	Validate();
}

//=============================================================================

void
DiskFileCD::SeekRandom( int32 position )
{
	Validate();
	assert( position >= 0 );
	assert( position < _MAX_SECTOR * _SECTOR_SIZE );
	assert(position <= _fileSize);
	assert( ( position % _SECTOR_SIZE ) == 0 );

	assert( mmio );
#if 1
	int delta = position - _currentFilePosition;
	mmioSeek( mmio, delta, SEEK_CUR );
#else
	mmioSeek( mmio, position, SEEK_SET );
#endif

	_currentFilePosition = position;
	Validate();
}

//=============================================================================

void
DiskFileCD::ReadBytes( void* buffer, int32 size )
{
	Validate();
	assert(size > 0);
	assert(ValidPtr(buffer));
	assert( ( _fileSize - _currentFilePosition ) >= size );
	assert( ( _currentFilePosition % _SECTOR_SIZE ) == 0 );
	assert( ( size % _SECTOR_SIZE ) == 0 );

#pragma message ("KTS " __FILE__ ": can this be removed?")
	SeekForward( _currentFilePosition );		// TEMP -- check hardware/software

	assert( mmio );
	int cbRead = mmioRead( mmio, (char*)buffer, size );
	AssertMsg( cbRead == size, "size=" << size << ", cbRead=" << cbRead );
	_currentFilePosition += size;
}

//=============================================================================

//=============================================================================
// DISKIO.cc:
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

#define _DISKFILE_CC

// if defined, build for cd streaming, otherwise build for hd streaming
//#define DO_CD_STREAMING

//=============================================================================

#include <hal/diskfile.hp>
#include <pigsys/pigsys.hp>
#include <cpplib/stdstrm.hp>

//=============================================================================

void
_DiskFile::Validate() const
{
	assert(_currentFilePosition >= 0);
	assert(_fileSize > 0);
	assert(_currentFilePosition <= _fileSize);
}

//=============================================================================

_DiskFile::_DiskFile( const char* )
{
}

//=============================================================================

_DiskFile::~_DiskFile()
{
	Validate();
}

//=============================================================================

_DiskFile*
ConstructDiskFile(char* filename,Memory& memory)		// factory function, creates correct type of diskfile
{
	assert(ValidPtr(filename));
	_DiskFile* file;
#if defined(DO_CD_STREAMING)
//	file = NEW( DiskFileCD( filename ) );
	file = new (memory) DiskFileCD( filename );
#else
//	file = NEW( DiskFileHD( filename ) );
	file = new (memory) DiskFileHD( filename );
#endif
	assert(ValidPtr(file));
	return file;
}

//=============================================================================

#if TEST_DISKFILE

char wholeFile[ DiskFileCD::_SECTOR_SIZE * 55 ];

#pragma message ("KTS " __FILE__ ": add some code to test SeekRandom (backward seeks)")


#include <iff/iffread.hp>

void
DiskFileTest(void)
{
	DiskFileHD cd( "dftest.iff" );
//	DiskFileCD cd( "DFTEST.IFF" );

	cd.SeekForward( 0 );
	cd.ReadBytes( (void*)wholeFile, sizeof( wholeFile ) );
	cd.SeekRandom( 0 );

	char buffer[ DiskFileCD::_SECTOR_SIZE ];

	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );

	cd.SeekRandom( 0 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	assert( ( *(int32*) buffer ) == int32( IFFTAG('L','V','A','S')) );

	cd.SeekForward( DiskFileCD::_SECTOR_SIZE );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	assert( (*(int32*) buffer ) == int32( IFFTAG('L','V','L','\0') ) );

	cd.SeekForward( 0x2800 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	assert( (*(int32*) buffer ) == int32( IFFTAG('P','E','R','M') ) );

	cd.SeekForward( 0x6800 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32*( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	printf("%x,%x\n",(*(int32*)buffer),int32(IFFTAG('R','M','0','\0')));
	assert( (*(int32*) buffer ) == int32( IFFTAG('R','M','0','\0') ) );

	cd.SeekForward( 0x14800 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	assert( (*(int32*) buffer ) == int32( IFFTAG('R','M','1','\0') ) );

	cd.SeekForward( 0x16000 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	assert( (*(int32*) buffer ) == int32( IFFTAG('R','M','2','\0') ) );

	cd.SeekForward( 0x1A000 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );
	assert( (*(int32*) buffer ) == int32( IFFTAG('R','M','3','\0') ) );

	cd.SeekRandom( 0 );
	cd.ReadBytes( (void*)buffer, DiskFileCD::_SECTOR_SIZE );
	//cout << int32( buffer ) << endl;
	buffer[ 4 ] = '\0';
	printf( "[%s]\n", buffer );

}
#endif

//=============================================================================

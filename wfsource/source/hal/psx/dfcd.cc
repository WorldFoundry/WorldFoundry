//=============================================================================
// DiskFileCD.cc:
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
// Original Author: Kevin T. Seghetti
//============================================================================

#define DO_HDUMP 0

//=============================================================================

#include <pigsys/pigsys.hp>
#include <hal/diskfile.hp>

extern "C" {
#	include <libgte.h>
#	include <libgpu.h>
#	include <libcd.h>
};

// since this is only used on an actuall psx, assertions are never on
#undef assert

//#define assert(exp) (void)( (exp) || (FatalError(#exp __FILE__ #__LINE__ ))
#define assert(exp) \
({\
	if(!(exp))\
	{\
		char buffer[100];\
		sprintf(buffer,"%s : %s: %d",#exp,__FILE__,__LINE__);\
		FatalError(buffer);\
	}\
})

//=============================================================================

extern void InitSimpleDisplay();
extern void UpdateSimpleDisplay();

void
HalInitFileSubsystem()
{
#if DO_HDUMP
	InitSimpleDisplay();
#endif
	CdInit();
	int error;
	error = CdControlB( CdlStop, 0, 0 );
	assert(error == 1);
#if DO_ASSERTIONS
 	CdSetDebug(2);		// CD debug level 2
#endif
#if DO_HDUMP
	FntPrint( "CDINIT" );
	UpdateSimpleDisplay();
#endif
}

//=============================================================================

void
DiskFileCD::Validate() const
{
	_DiskFile::Validate();
}

//=============================================================================

DiskFileCD::DiskFileCD( const char* fileName ) : _DiskFile( fileName )
{
	printf("filename = %s\n",fileName);
	assert( fileName );
	assert( strlen( fileName ) < _MAX_PATH );

	// Stop the CD and sync to it
	int error;
	error = CdControlB( CdlStop, 0, 0 );
	assert(error == 1);
	error = CdSync( 0, 0 );
	assert(error != CdlDiskError);

	// set double speed mode
	u_char param[4];
	param[0] = CdlModeSpeed;
	error = CdControl( CdlSetmode, param, 0 );
	assert(error == 1);

	// Convert offset to a long
	_fileStartPos = 23 * _SECTOR_SIZE;
	assert( _fileStartPos >= 0 );

#if DO_ASSERTIONS
	_fileSize = 389120;
	assert( _fileSize > 0 );
#endif

	_currentFilePosition = 0;

#pragma message ("KTS " __FILE__ ": is this needed?")
	SeekForward( 0 );				// Seek to the beginning of the file -- needed?

	CdSetDebug( 1 );		// CD debug level 2
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
#if DO_HDUMP
#pragma message( "remove" )
	char szMsg[ 80 ];
	sprintf( szMsg, "SeekRandom: pos=%ld sector=%d,_fileStartPos = %d\n", position, position / _SECTOR_SIZE, _fileStartPos );
	FntPrint( szMsg );
	UpdateSimpleDisplay();
#endif
	Validate();
	assert( position >= 0 );
	assert( position < _MAX_SECTOR * _SECTOR_SIZE );
	assert( position % DiskFileCD::_SECTOR_SIZE == 0 );
//	assert(position <= _fileSize);

	CdlLOC pos;
	int error;
	CdIntToPos( (_fileStartPos + position) / _SECTOR_SIZE, &pos );				// Same track as file
	error = CdControlB( CdlSetloc, (u_char*)&pos, 0 );	// Seek from the beginning of the file
	assert(error == 1);

	_currentFilePosition = position;
	Validate();

//	DBSTREAM1( cout << "SeekRandom( " << position << " ) complete" << endl; )
}

//=============================================================================

void
HDump(char* title, void* buffer, ulong bufferSize);

void
DiskFileCD::ReadBytes( void* buffer, int32 size )
{
#if DO_HDUMP
#pragma message( "remove" )
	char szMsg[ 80 ];
	sprintf( szMsg, "ReadBytes: size=%ld, buffer = %p\n", size, buffer );
	FntPrint( szMsg );
	UpdateSimpleDisplay();
#endif
	Validate();
	assert(size > 0);
	assert(ValidPtr(buffer));
	AssertMsg( ( _fileSize - _currentFilePosition ) >= size,
		"_fileSize = " << _fileSize << " _currentFilePosition = " << _currentFilePosition << "size = " << size
	);

	//?Seek( _currentFilePosition );		// TEMP -- check hardware/software

	CdReadCallback( NULL );

	assert( ( size % _SECTOR_SIZE ) == 0 );

	int error = CdRead( size / _SECTOR_SIZE, (u_long*)buffer, CdlModeSpeed );
	assert(error == 1);
//	assert( cbRead == size );
	_currentFilePosition += size;

	do
	{
		error = CdReadSync( 1, 0 );
//		printf("CdReadSync retcode = %d\n",error);
		assert(error != -1);
	} while(error > 0);

#if DO_HDUMP
	HDump("Sector Dump:",((char*)buffer)-36, size);
#endif
}

//=============================================================================

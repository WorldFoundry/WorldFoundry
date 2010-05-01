//==============================================================================
// ruvdump.cc
// By William B. Norris IV
// Copyright (c) 1995-1999, World Foundry Group  
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
//==============================================================================

#include "global.hp"

#include <stdio.h>
#include <pigsys/assert.hp>
#include <string>
#include <stdlib.h>
#include <iostream>

#include "rmuv.hp"
//#include "../brender/inc/brender.h"

//#include "memcheck.h"


long
filesize( FILE* fp )
{
	long save_pos, size_of_file;

	save_pos = ftell( fp );
	fseek( fp, 0L, SEEK_END );
	size_of_file = ftell( fp );
	fseek( fp, save_pos, SEEK_SET );
	return size_of_file;
}


const char*
printpair( int i1, int i2 )
{
	static char szBuffer[ 30 ];
	sprintf( szBuffer, "(%d,%d)", i1, i2 );
	return szBuffer;
}


void
ruvdump( const char* filename )
{
	printf( "%s: ", filename );

	FILE* fp = fopen( filename, "rb" );
	if ( !fp )
	{
		cerr << "Unable to open \"" << filename << "\" -- Aborting" << endl;
		exit( 1 );
	}

	long len = filesize( fp );
	char* pRuv = (char*)malloc( len );
	char* ruv = pRuv;
	assert( ruv );
	fread( ruv, len, 1, fp );
	fclose( fp );

#if 0
	{ // Test RMUV class
	RMUV rmuv( (void*)ruv );

	cout << filename << " has " << rmuv.NumEntries() << " entries" << endl;
	_RMUV* bmp24 = rmuv.GetRMUV( "24.bmp" );
	assert( bmp24 );
	cout << *bmp24 << endl;
	}
#endif

	// Check for RMUV signature

	ruv += 8;
	int nEntries = *((long*)ruv);
	printf( "%ld entries\n", nEntries );
	ruv += 4;

    printf( "ษออออออออออออหอออออัอออออออออัอออออออออัอออออัอออออัอออออออออออป\n" );
    printf( "บName        บFrameณ  (u,v)  ณ  (w,h)  ณ#bitsณTransณ(palx,paly)บ\n" );
    printf( "ฬออออออออออออฮอออออุอออออออออุอออออออออุอออออุอออออุอออออออออออน\n" );
	_RMUV* rmuv = (_RMUV*)ruv;
	for ( int i=0; i<nEntries; ++i, rmuv += 1 )
	{
        printf( "บ%-12sบ  %01X  ณ%9sณ",
			rmuv->szTextureName,
			rmuv->nFrame,
			printpair( rmuv->u, rmuv->v ) );
        printf( "%9sณ%4d ณ", printpair( rmuv->w, rmuv->h ), rmuv->bitdepth );
		printf( "%5sณ", rmuv->bTranslucent ? "true" : "false" );
        printf( "%11sบ\n", printpair( rmuv->palx, rmuv->paly ) );
	}

    printf( "ศออออออออออออสอออออฯอออออออออฯอออออออออฯอออออฯอออออฯอออออออออออผ\n" );

	free( pRuv );
}


int
main( int argc, char* argv[] )
{
	cout << "RUVDump v0.2 [ sizeof( _RMUV ) = " << sizeof( _RMUV ) << " ]" << endl;

	if ( argc == 1 )
	{
		char* cp;
		if ( cp = strrchr( argv[0], '\\' ) )
			++cp;
		else
			cp = argv[ 0 ];
		cout << "Usage : " << ((cp) ? cp : argv[0]) << " {switches} <.ruv file>" << endl;
		exit(1);
	}

	for ( int i=1; i<argc; ++i )
		ruvdump( argv[ i ] );

	return 0;
}

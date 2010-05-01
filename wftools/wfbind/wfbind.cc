//==============================================================================
// wfbind.cc
// Copyright (c) 1997-1999, World Foundry Group  
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

#define STRICT
#include <windows.h>
#include <cstdio>
#include <pigsys/assert.hp>
#include <loadfile/loadfile.hp>

int
main( int argc, char* argv[] )
{
	if ( argc < 3 )
	{
		printf( "wfbind -- Bind levels files to a World Foundry Engine\n" );
                printf( "Copyright 1998,1999 William B. Norris IV.  All Rights Reserved.\n" );
		printf( "Usage: wfbind <exe> <data> [<tagString>]\n" );
		return 10;
	}

	FILE* fp = fopen( argv[ 1 ], "a+b" );
	assert( fp );

        int32 cbDataFile;
	char* pData = (char*)LoadBinaryFile( argv[ 2 ], cbDataFile );
	assert( pData );

	fwrite( pData, cbDataFile, 1, fp );
        // TODO: add assert

	fwrite( &cbDataFile, 4, 1, fp );
        // TODO: add assert

	if ( argc == 4 )
	{
		int cbWritten = fwrite( argv[ 3 ], 1, strlen( argv[ 3 ] ), fp );
		assert( cbWritten == strlen( argv[ 3 ] ) );
	}

	assert( pData );
	free( pData );

	fclose( fp );

	return 0;
}

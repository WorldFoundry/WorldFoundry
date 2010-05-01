//=============================================================================
// loadfile.cc
// Copyright ( c ) 1997,99 World Foundry Group  
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
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of World Foundry Group.
// The contents of this file may not be disclosed to third parties, copied
// or duplicated in any form, in whole or in part, without the prior written
// permission of World Foundry Group
// ===========================================================================
// History:
// ?? ??? ??	WBNIV	Created
//============================================================================

#include <cstdio>
#include <pigsys/assert.hp>
#include <iostream>
#include <cstdlib>

static long
ffilesize( FILE *fp )
{
	assert( fp );

	long lSavedPos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	long lFileSize = ftell( fp );
	fseek( fp, lSavedPos, SEEK_SET );

	return lFileSize;
}


void*
LoadBinaryFile( const char* szFilename, unsigned long &sizeOfFile )
{
	assert( szFilename );

	FILE* fp = fopen( szFilename, "rb" );
	if ( !fp )
	{
		cerr << "Unable to open input file \"" << szFilename << "\"" << endl;
		return NULL;
	}

	sizeOfFile = ffilesize( fp );
	unsigned long* dataFile = (unsigned long*)malloc( sizeOfFile );
	int nBytesRead = fread( dataFile, 1, sizeOfFile, fp );
	assert( nBytesRead == sizeOfFile );

	fclose( fp );

	return dataFile;
}

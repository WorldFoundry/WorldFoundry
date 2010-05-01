//==============================================================================
// xcd.cc
// Copyright (c) 1996-1999, World Foundry Group  
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

#include <stdlib.h>
#include <pigsys/assert.hp>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#if __WIN__
#       include <dos.h>
#endif

const char szTagName[] = "name=";
const char szTagDirectory[] = "dir=";

void
xcd_listDir( const char* szDirList )
{
	printf( "%s:\n", szDirList );

	int fh = open( szDirList, O_RDWR | O_TEXT );
	if ( fh == -1 )
        {
		printf( "No xcd entries exist\n" );
		return;
        }

	int length = filelength( fh );
	assert( length != -1 );

	char* dirs = (char*)malloc( length+1 );
	assert( dirs );

	length = read( fh, (void*)dirs, length );
	*( dirs + length ) = '\0';
	strlwr( dirs );

	puts( dirs );

	free( dirs );

	close( fh );
}


void
xcd_addDir( const char* szDirList, const char* _szName )
{
	char* szCurrentDir;
	szCurrentDir = getcwd( NULL, 0 );
	assert( szCurrentDir );
	strlwr( szCurrentDir );

        int fh = open( szDirList, O_RDWR | O_APPEND | O_CREAT
#if defined( __UNX__ )
                , S_IRWXU | S_IRWXG | S_IRWXO
#endif
                );
	assert( fh != -1 );

	char szName[ _MAX_PATH ];
	if ( _szName )
		strcpy( szName, _szName );
	else
        {
		_splitpath( szCurrentDir, NULL, NULL, szName, NULL );
		strlwr( szName );
        }

	char szEntry[ _MAX_PATH * 2 ];
        sprintf( szEntry, "%s%s\t%s%s\n", szTagName, szName, szTagDirectory, szCurrentDir );
	printf( "Entry \"%s\" for directory \"%s\" added\n", szName, szCurrentDir );
	write( fh, szEntry, strlen( szEntry ) );

	close( fh );

	free( szCurrentDir );
}


void
_xcd( const char* szDirList, const char* _szLookFor )
{
	assert( _szLookFor );

	int fh = open( szDirList, O_RDWR | O_TEXT );
	if ( fh == -1 )
        {
		printf( "No xcd entries exist\n" );
		return;
        }

	int length = filelength( fh );
	assert( length != -1 );

	char* dirs = (char*)malloc( length+1 );
	assert( dirs );

	length = read( fh, (void*)dirs, length );
	*( dirs + length ) = '\0';
	strlwr( dirs );

	char szLookForName[ 256 ];
        strcpy( szLookForName, szTagName );
	strcat( szLookForName, _szLookFor );
	strlwr( szLookForName );

	char* line = strstr( dirs, szLookForName );
	if ( line )
        {
		// Now find the EOL char and NUL it out to zero-terminate the string
		char* pchEOL = strchr( line, '\n' );
		if ( *pchEOL )
			*pchEOL = '\0';

//		printf( "Found: [%s]\n", line );

                char* szDirName = strstr( line, szTagDirectory );
                szDirName += strlen( szTagDirectory );
//		printf( "Changing to directory [%s]\n", szDirName );
		int ret = chdir( szDirName );
		printf( "chdir() = %d\n", ret );
#if defined( __DOS__ )
		unsigned int nDrives;		// don't actually care
		_dos_setdrive( *szDirName - 'a' + 1, &nDrives );
#endif
        }

	free( dirs );
}


int
main( int argc, char* argv[] )
{
	char szDrive[ _MAX_DRIVE ];
	char szPath[ _MAX_PATH ];

	_splitpath( argv[0], szDrive, szPath, NULL, NULL );

	char szDirList[ _MAX_PATH ];
#if __UNX__
        strcpy( szDirList, "~/.xcd" );
#else
        _makepath( szDirList, szDrive, szPath, ".xcd", "" );
#endif

        if ( argc == 1          // only program name given
        || argc > 3 )           // too many parameters
        {
                printf( "xcd  Extended Change Directory v1.1\n"
                        "Copyright 1995,1999 William B. Norris IV.  All Rights Reserved.\n"
		        "\n"
		        "Usage:\txcd <name> |\n"
				"\txcd + [<name>]\tCurrent directory name will be used if none given\n"
				"\n" );

		xcd_listDir( szDirList );

		return 1;
        }

	if ( strcmp( argv[1], "+" ) == 0 )
		xcd_addDir( szDirList, argv[2] );
	else
		_xcd( szDirList, argv[1] );

	return 0;
}

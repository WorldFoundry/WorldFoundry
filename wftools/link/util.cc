//==============================================================================
// util.cc
// Copyright (c) 1998-1999, World Foundry Group  
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
#include <stdio.h>
#include <pigsys/assert.hp>
#include <string.h>
#include "util.hp"

long
	ffilesize( FILE *fp )
	{
	assert( fp );

	long lSavedPos = ftell( fp );
	fseek( fp, 0, SEEK_END );
	long lFileSize = ftell( fp );
	fseek( fp, lSavedPos, SEEK_SET );

	return( lFileSize );
	}


bool
fileExists( const char* szFilename )
	{
	FILE* fp = fopen( szFilename, "r" );
	if ( fp )
		{
		fclose( fp );
		return true;
		}
	else
		return false;
	}


void*
LoadBinaryFile( const char* _szFilename, int& nSize )
	{
	FILE* fp;
	void* ptr;
	const char* szFilename = _szFilename;

	fp = fopen( (char*)szFilename, "rb" );
	if ( !fp )
		return NULL;

	fseek( fp, 0L, SEEK_END );
	assert( !ferror( fp ) );
	nSize = ftell( fp );
	fseek( fp, 0L, SEEK_SET );
	assert( !ferror( fp ) );

	ptr = malloc( nSize );
	assert( ptr );
	if ( ptr )
		{
		int cbRead = fread( ptr, 1, nSize, fp );
		assert( !ferror( fp ) );
		assert( cbRead == nSize );
		}

	fclose( fp );
	assert( !ferror( fp ) );

	return ptr;
	}


char*
LoadTextFile( const char* _szFilename, int& nSize )
	{
	FILE* fp;
	char* ptr;
	const char* szFilename = _szFilename;

	fp = fopen( (char*)szFilename, "rt" );
	if ( !fp )
		return NULL;

	fseek( fp, 0L, SEEK_END );
	assert( !ferror( fp ) );
	nSize = ftell( fp );
	fseek( fp, 0L, SEEK_SET );
	assert( !ferror( fp ) );

	ptr = (char*)malloc( nSize+1 );
	assert( ptr );
	if ( ptr )
		{
		int cbRead = fread( ptr, 1, nSize, fp );
		assert( !ferror( fp ) );
		assert( cbRead <= nSize );
		*( ptr + cbRead ) = '\0';
		nSize = cbRead;
		}

	fclose( fp );
	assert( !ferror( fp ) );

	return ptr;
	}


char*
strchrs( const char* s, const char* charset )
	{
	char* szFollow = (char*)s;

	while ( *szFollow && !strchr( charset, *szFollow ) )
		++szFollow;

	return *szFollow && strchr( charset, *szFollow ) ? szFollow : NULL;
	}
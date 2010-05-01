// ------------------------------------------------------------------------
// Copyright (c) 1995, PF. Magic, Inc.  All Rights Reserved.
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of PF. Magic, Inc.;
// The contents of this file may not be disclosed to third parties, copied
// or duplicated in any form, in whole or in part, without the prior
// written permission of PF. Magic, Inc.
// ------------------------------------------------------------------------

#include <pigsys/pigsys.hp>
#include <pigsys/assert.hp>
#include <cpplib/stdstrm.hp>
#include "fpath.hp"
#include <ctype.h>
#include <string.h>

// ------------------------------------------------------------------------

void
StrCpySub( char * dest, const char * source, int n )
{
	strncpy( dest, source, n );
	*(dest + n) = '\0';
}

// ------------------------------------------------------------------------
// INPUT PARAMETERS
// path		full file name to be split
// OUTPUT PARAMETERS
// drive	buffer to hold drive letter followed by a colon or empty string
// dir		buffer to hold path with a trailing slash or the empty string
// fname	buffer to hold filename without dribe, dir, or ext
// ext		buffer to hold extension with preceding period or empty string


#define INVALID_POS -1

void
_splitpath( const char * path, char * drive, char * dir,
	char * fname, char * ext )
{
	int begin, end, drivebegin, dirbegin, filebegin, extbegin, i;

	if( NULL == path ) return;			// null path

	// find the begin and end (strip out whitespace)
	begin = 0;
	end = strlen( path );
	while( isspace( path[begin] ) )
	{
		begin++;
		if( begin == end ) return;		// empty path
	}
	while( ( end >= begin ) && isspace( path[end - 1] ) ) end--;

	// find the end of the path and extension
	filebegin = end - 1;
	extbegin = INVALID_POS;
	while( filebegin >= begin 
           && path[filebegin] != '/' 
          && path[filebegin] != '\\' )
	{
		if( path[filebegin] == '.' ) extbegin = filebegin;
        filebegin--;
	}
	filebegin++;
    assert(filebegin < 65536);      // kts arbitrary

	// find the dir begin and drive begins
	drivebegin = INVALID_POS;
	if( filebegin == begin )
	{
		dirbegin = INVALID_POS;
	}
	else
	{
		i = begin;
		dirbegin = begin;
		while( i < filebegin )
		{
			if( path[i] == ':' )
			{
				drivebegin = begin;
				if( i + 1 < filebegin ) dirbegin = i + 1;
				else dirbegin = INVALID_POS;
				break;
			}
			i++;
		}
	}

	// error checking
	assert( filebegin != INVALID_POS );
	//assert( begin >= 0 && filebegin >= begin );
	assert( filebegin >= begin );
	assert( end <= (signed int)strlen( path ) );
	assert( drivebegin == INVALID_POS ||
			( ( drivebegin >= begin ) && (
		    	( ( dirbegin != INVALID_POS ) && ( drivebegin < dirbegin ) ) ||
		    	( drivebegin < filebegin ) ) ) );
	assert( dirbegin == INVALID_POS ||
		    ( ( dirbegin >= begin ) && ( dirbegin < filebegin ) ) );
	assert( extbegin == INVALID_POS ||
			( ( extbegin >= begin ) && ( filebegin < extbegin ) ) );

	// fill ext and file buffers
	if( extbegin != INVALID_POS )
	{
		if( NULL != fname )
			StrCpySub( fname, path + filebegin, extbegin - filebegin );
		if( NULL != ext )
			StrCpySub( ext, path + extbegin, end - extbegin );
	}
	else
	{
		if( NULL != fname )
			StrCpySub( fname, path + filebegin, end - filebegin );
		if( NULL != ext )
			strcpy( ext, "" );
	}

	// fill dir and drive buffers
	if( dirbegin == INVALID_POS )
	{
		if( NULL != dir ) strcpy( dir, "" );
		if( NULL != drive ) {
			if( drivebegin == INVALID_POS ) strcpy( drive, "" );
			else StrCpySub( drive, path + drivebegin, filebegin - drivebegin );
		}
	}
	else // dirbegin != INVALID_POS
	{
		if( NULL != dir )
			StrCpySub( dir, path + dirbegin, filebegin - dirbegin );
		if( NULL != drive ) {
			if( drivebegin == INVALID_POS ) strcpy( drive, "" );
			else StrCpySub( drive, path + drivebegin, dirbegin - drivebegin );
		}
	}
}

// ------------------------------------------------------------------------
// OUTPUT PARAMETERS
// path		buffer to hold full file name
// INPUT PARAMETERS
// drive	drive letter followed by a colon or empty string
// dir		path with or without a trailing slash or the empty string
// fname	filename without drive, dir, or ext
// ext		extension with preceding period or empty string

void
_makepath( char * path, const char * drive, const char * dir,
	const char * fname, const char * ext )
{
	int i;

	*path = '\0';
	if( drive != NULL && drive[0] != '\0' )
		strcat( path, drive );
	if( dir != NULL && dir[0] != '\0' )
	{
		strcat( path, dir );
		i = strlen( dir ) - 1;
		while( i >= 0 && isspace( dir[i] ) ) i--;
		if( i >= 0 && !( dir[i] == '/' || dir[i] == '\\' ) )
			strcat( path, "\\" );
	}
	if( fname != NULL && fname[0] != '\0' )
		strcat( path, fname );
	if( ext != NULL && ext[0] != '\0' )
	{
		if( ext[0] != '.' ) strcat( path, "." );
		strcat( path, ext );
	}
}

// ------------------------------------------------------------------------
// INPUT PARAMETERS
// source		a string to decompose into tokens
// delim		a list of characters that are delimiters
// OUTPUT PARAMETERS
// tokenbuf		a buffer to hold the extracted token
// nextdelim	a ptr to the delimiter that caused the termination of tokenbuf
// RETURN VALUE
// ptr to start of token in string or NULL

char *
StrTokenize( char * source, const char * delim, char * tokenbuf,
	char ** nextdelim )
{
	char * srcstr, * next;

	assert( NULL != delim && delim[0] != '\0' );
	assert( NULL != source );

	// find beginning of token (if exists)
	srcstr = source;
	if( *srcstr == '\0' ) return NULL;
	while( strchr( delim, *srcstr ) )
	{
		 srcstr++;
		 if( *srcstr == '\0' ) return NULL;
	}

	// find end of token
	next = srcstr;
	while( !strchr( delim, *next ) )
	{
		if( *next == '\0' ) break;
		next++;
	}
	*nextdelim = next;
	assert( next >= source && next <= source + strlen( source ) );

	// construct token
	StrCpySub( tokenbuf, srcstr, next - srcstr );

	return srcstr;
}

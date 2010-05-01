
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string>

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

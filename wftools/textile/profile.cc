//==============================================================================
// profile.cc:
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
#include <stdlib.h>
#include <stdio.h>
//#include <io.h>
#include <string>
#include <ctype.h>

// Support Routines
static void strip_crlf(char *);
static void strip_ws(char *);
static bool FindLine(FILE *, const char *, const char *, char *, FILE *);
static bool FindApp(FILE *, const char *, FILE *);

static void strip_crlf( char *s )
{
    char *c;

    if ( (c=strrchr(s, 10))!=0 ) *c = '\0';
    if ( (c=strrchr(s, 13))!=0 ) *c = '\0';
}

#if 0
static
	void strip_ws( char *s )
	{
	char *write, *look;

	for ( write=look=s; *look; ++look )
		{
		if ( !isspace( *look ) )
			*write++ = *look;
		}

	*write = '\0';
	}
#endif

#if 0
static void
strip_ws( char *s )
	{
	char *write, *look;

	for ( write = look = s; *look; ++look )	{
		if ( !isspace( *look ) ) {
			if ( *look == '_' ) {
				*look = ' ';
				}
			*write++ = *look;
			}
		}
	*write = '\0';
	}

static void
unstrip_ws( char *s )
	{
	char *write;

	for ( write = s; *write; ++write )	{
		if ( *write == ' ' ) {
			*write = '_';
			}
		}
	}
#endif

static bool
FindApp( FILE *fp, const char *lpApplicationName, FILE *fpo )
	{
	char szMatch[100];
	char szBuff[4096];
        bool bFoundApp = false;


	sprintf( szMatch, "[%s]", lpApplicationName );

	rewind( fp );

	while ( ( !feof( fp ) ) && ( fgets( szBuff, 4096, fp ) ) )
		{
		if ( fpo )
			{
			fprintf( fpo, szBuff );
			}

		strip_crlf( szBuff );

		if ( stricmp( szMatch, szBuff ) == 0 )
			{
                        bFoundApp = true;
			break;
			}
		}
	return( bFoundApp );
	}


static bool
FindLine( FILE *fp, const char * lpKeyName, const char * lpApplicationName, char *szLine, FILE *fpo )
	{
	char szMatch[100];
	char szBuff[4096];
	char szOriginal[4096];

        bool bFoundKeyname = false;

	*szLine = '\0';                 // Nothing found (default)

	if ( FindApp( fp, lpApplicationName, fpo ) )
		{ // Look for KeyName
		while ( ( !feof( fp ) ) && ( fgets( szBuff, 4096, fp ) ) )
			{
			strcpy( szOriginal, szBuff );

			strip_crlf( szBuff );

			//strip_ws( szBuff );

			if ( strnicmp( szBuff, lpKeyName, strlen( lpKeyName ) ) == 0 )
				{ // Found the match!
                                bFoundKeyname = true;
				strcpy( szLine, szBuff );
				break;
				}

			if ( fpo )
				{
				fprintf( fpo, szOriginal );
				}

			if ( *szBuff == '[' )
				// Another application section.  Didn't find it.
				break;

			}
		}

	return( bFoundKeyname );
	}


/*
 * This function retrieves the value of an integer key from the specified
 * initialization file.
 *
 * The function searches the file for a key that matches the name specified
 * by the lpKeyName parameter under the application heading specified by the
 * lpApplicationName parameter.  An integer entry in the initialization file
 * must have the following form:
 *
 * [application name]
 * keyname = value
 *  .
 *  .
 *  .
 *
 * The return value specifies the result of the function.  The return value
 * is zero if the value that corresponds to the specified key name is not
 * an integer or if the integer is negative.  If the value that corresponds
 * to the key name consists of digits followed by nonnumeric characters, the
 * function returns the value of the digits.  For example, if the entry
 * KeyName = 102abc is accessed, the function returns 102.  If the key is
 * not found, this function returns the default value, nDefault.
 *
 * The GetPrivateProfileInt function is not case dependent, so the strings
 * in lpApplicationName and lpKeyName may be in any combination of uppercase
 * and lowercase letters.
 *
 */
int
GetPrivateProfileInt( const char* lpApplicationName, const char* lpKeyName, int nDefault, const char* lpFileName )
	{
	FILE *fp;
	char szLine[4096];
	int nReturn = nDefault;

	if ( (fp = fopen( lpFileName, "rt" ) ) !=0)
		{
		if ( FindLine( fp, lpKeyName, lpApplicationName, szLine, NULL ) )
			{
			char *szEquals;

			if ( ( szEquals = strchr( szLine, '=' ) )!=0)
				{
				nReturn = atoi( szEquals+1 );
				}
			}
		fclose( fp );
		}

	return( nReturn );
	}


/*
 * GetPrivateProfileString(
 *
 * This function copies a character string from the specified initialization
 * file into the buffer pointed to be the lpReturnedString parameter.
 *
 * The function searches the file for a key that matches the name specified
 * by the lpKeyName parameter under the application heading specified by the
 * lpApplicationName parameter.  If the key is found, the corresponding
 * string is copied to the buffer.  If the key does not exist, the default
 * character string specified by the lpDefault parameter is copied.  A string
 * entry in the initialization file must have the following form:
 *
 * [application name]
 * keyname = string
 *   .
 *   .
 *   .
 *
 * If lpKeyName is NULL, the GetPrivateProfileString function enumerates all
 * key names associated with lpApplicationName by filling the location
 * pointed to by lpReturnedString with a list of key names (not values).
 * Each key name in the list is terminated with a null character.
 *
 * The return value specifies the number of characters copied to the buffer
 * identified by the lpReturnedString parameter, not including the null
 * character.  If the buffer is not large enough to contain the entire
 * string and lpKeyName is not NULL, the return value is equal to the length
 * specified by the nSize parameter.  If the buffer is not large enough to
 * contain the entire string and lpKeyName is NULL, the return value is equal
 * to the length specified by the nSize parameter minus 2.
 *
 * GetPrivateProfileString is not case dependent, so the strings in the
 * lpApplicationName and lpKeyName may be in any combination of uppercase
 * and lowercase letters.
 *
 */
int
GetPrivateProfileString( const char* lpApplicationName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, int nSize, const char* lpFileName )
	{
	FILE *fp;
	char szLine[4096];
	int nReturn;

	strncpy( lpReturnedString, lpDefault, nSize );      // Default return

	if ( ( fp = fopen( lpFileName, "rt" ) )!=0)
		{
		if ( FindLine( fp, lpKeyName, lpApplicationName, szLine, NULL ) )
			{
			char *szEquals;

			if (( szEquals = strchr( szLine, '=' ) )!=0)
				{
				++szEquals;			// skip past '='
				while ( isspace( *szEquals ) )
					++szEquals;
				strncpy( lpReturnedString, szEquals, nSize );
				}
			}
		fclose( fp );
		}

	nReturn = strlen( lpReturnedString );
	return( nReturn );
	}


/*
 * This function copies the character string pointed to by the lpString
 * parameter into the specified initalization file.  It searches the file
 * for the key named by the lpKeyName parameter under the application
 * specified by the lpApplicationName parameter.  If there is no match, it
 * adds to the user profile a new string entry containing the key name and
 * the key value specified by the lpString parameter.  If there is a
 * matching key, the function replaces that key's value with lpString.
 *
 * The return value specifies the result of the function.  It is nonzero
 * if the function is successful.  Otherwise, it is zero.
 *
 * If there is no application field for lpApplicationName, this function
 * creates a new application field and places an appropriatee key-value
 * line in that field of the initialization file.
 *
 * A string entry in the initialization file has the following form:
 *
 * [application name]
 * keyname = string
 *   .
 *   .
 *   .
 *
 * An application can also call WritePrivateProfileString to delete lines...
 *
 */
bool
WritePrivateProfileString( const char *lpApplicationName, const char *lpKeyName, const char *lpString, const char *lpFileName )
	{
	FILE *fp;
	FILE *fpo;
	char szTempName[13];
	char szBuff[4096];

	if (( fp = fopen( lpFileName, "a+t" ) )!=0)
		{
		tmpnam( szTempName );
		fpo = fopen( szTempName, "wt" );

		if ( !FindApp( fp, lpApplicationName, fpo ) )
			{ // Create a new application field
			fseek( fpo, 0, SEEK_END );
			fprintf( fpo, "\n[%s]\n", lpApplicationName );
			fprintf( fpo, "%s = %s\n", lpKeyName, lpString );
			}
		else
			{
			rewind( fpo );
			FindLine( fp, lpKeyName, lpApplicationName, szBuff, fpo );

			// Add replacement (or new) line
			strncpy( szBuff, lpString, 255 );
			//unstrip_ws( szBuff );
			fprintf( fpo, "%s = %s\n", lpKeyName, szBuff );
	//		fprintf( fpo, "%s = %s\n", lpKeyName, lpString );

			// Read remainder of file and write out to new file
			while ( ( !feof(fp) ) && ( fgets( szBuff, 255, fp ) ) )
				{
				fprintf( fpo, szBuff );
				}
			}
		fclose( fpo );
		fclose( fp );

		unlink( lpFileName );
		rename( szTempName, lpFileName );
		}

	return 1;
	}


bool
WritePrivateProfileInt( const char *lpApplicationName, const char *lpKeyName, int nValue, const char *lpFileName )
	{
	char szBuff[4096];

	ftell(NULL);
	sprintf( szBuff, "%d", nValue );
	return(
		WritePrivateProfileString( lpApplicationName, lpKeyName, szBuff, lpFileName )
		);
	}


/*const*/ char szSystemIni[] = "e:\\dr\\bin";

int
GetProfileInt( char* lpszSection, char* lpszEntry, int nDefault )
	{
	return( GetPrivateProfileInt( lpszSection, lpszEntry, nDefault,
		szSystemIni ) );
	}


int
GetProfileString( char *lpApplicationName, char *lpKeyName, char *lpDefault, char *lpReturnedString, int nSize )
	{
	return( GetPrivateProfileString( lpApplicationName, lpKeyName, lpDefault,
		lpReturnedString, nSize, szSystemIni ) );
	}


bool
WriteProfileString( char *lpApplicationName, char *lpKeyName, char *lpString )
	{
	return( WritePrivateProfileString( lpApplicationName, lpKeyName,
		lpString, szSystemIni ) );
	}


bool
WriteProfileInt( char *lpApplicationName, char *lpKeyName, int nValue )
	{
	return( WritePrivateProfileInt( lpApplicationName, lpKeyName, nValue,
		szSystemIni ) );
	}

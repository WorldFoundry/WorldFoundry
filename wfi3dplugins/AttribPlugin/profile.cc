//==============================================================================
// profile.cc:
//==============================================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/io.h>
#include <limits.h>


#define _MAX_PATH _POSIX_PATH_MAX

char
UpperCase(char input)
{
    if(input >= 'a' && input <= 'z')
        input += ('a'-'A');
    return input;
}

int
_stricmp(const char* left, const char* right)
{
    char lchar;
    char rchar;
    while(1)
    {
        lchar = UpperCase(*left);
        rchar = UpperCase(*right);
        
        if(lchar == rchar)
        {
            left++;
            right++;
            if(lchar == 0)
                return 0;
        }
        else
            if(lchar < rchar)
                return -1;
            else
                return 1;
    }
}

int
_strnicmp(const char* left, const char* right,int len)
{
    char lchar;
    char rchar;
    while(len--)
    {
        lchar = UpperCase(*left);
        rchar = UpperCase(*right);
        
        if(lchar == rchar)
        {
            left++;
            right++;
            if(left == 0)
                return 0;
        }
        else
            if(lchar < rchar)
                return -1;
            else
                return 1;
    }
    return 0;
}

//#include "general.hp"
#include <profile.hp>

const int BUF_LEN = 4096;

// Support Routines
static void strip_crlf(char *);
//static void strip_ws(char *); Never used, all commented out, never defined.
static bool FindLine(FILE *, const char *, const char *, char *, int bufferSize, FILE *);
static bool FindApp(FILE *, const char *, FILE *);

static void
strip_crlf( char *s )
{
	char *c;

	if ( (c=strrchr(s, 10))!=0 ) *c = '\0';
	if ( (c=strrchr(s, 13))!=0 ) *c = '\0';
}


static bool
FindApp( FILE *fp, const char *lpApplicationName, FILE *fpo )
{
	char szMatch[100];
	char szBuff[BUF_LEN];
	bool bFoundApp = false;


	sprintf( szMatch, "[%s]", lpApplicationName );

	rewind( fp );

	while ( ( !feof( fp ) ) && ( fgets( szBuff, sizeof( szBuff ), fp ) ) )
		{
		if ( fpo )
			{
			fprintf( fpo, szBuff );
			}

		strip_crlf( szBuff );

		if ( _stricmp( szMatch, szBuff ) == 0 )
			{
			bFoundApp = true;
			break;
			}
		}
	return( bFoundApp );
}


static bool
FindLine( FILE *fp, const char * lpKeyName, const char * lpApplicationName, char *szLine, int bufferSize, FILE *fpo )
{
	char szBuff[BUF_LEN];
	char szOriginal[BUF_LEN];

	bool bFoundKeyname = false;

	*szLine = '\0';                 // Nothing found (default)

	if ( FindApp( fp, lpApplicationName, fpo ) )
		{ // Look for KeyName
		while ( ( !feof( fp ) ) && ( fgets( szBuff, sizeof( szBuff ), fp ) ) )
			{
			strcpy( szOriginal, szBuff );

			strip_crlf( szBuff );

//                        strip_ws( szBuff );

			if ( _strnicmp( szBuff, lpKeyName, strlen( lpKeyName ) ) == 0 )
				{ // Found the match!
				bFoundKeyname = true;
				strncpy( szLine, szBuff, bufferSize );
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
GetPrivateProfileInt(const char* lpApplicationName, const char* lpKeyName, int nDefault, const char* lpFileName)
{
	FILE *fp;
	char szLine[BUF_LEN];
	int nReturn = nDefault;

	if ( (fp = fopen( lpFileName, "r+b" ) ) !=0)
		{
		if ( FindLine( fp, lpKeyName, lpApplicationName, szLine, BUF_LEN, NULL ) )
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
 * If the value is not found the return value = -1
 *
 * GetPrivateProfileString is not case dependent, so the strings in the
 * lpApplicationName and lpKeyName may be in any combination of uppercase
 * and lowercase letters.
 *
 */
int
GetPrivateProfileString(const char* lpApplicationName, const char* lpKeyName, const char* lpDefault, char* lpReturnedString, int nSize, const char* lpFileName )
{
	FILE *fp;
	char szLine[BUF_LEN];
	int nReturn = -1;

	strncpy( lpReturnedString, lpDefault, nSize );      // Default return

	if ( ( fp = fopen( lpFileName, "r+b" ) )!=0)
		{
		if ( FindLine( fp, lpKeyName, lpApplicationName, szLine, BUF_LEN, NULL ) )
			{
			char *szEquals;

			if (( szEquals = strchr( szLine, '=' ) )!=0)
				{
				++szEquals;			// skip '='
				// Find first non-whitespace character
				while ( isspace( *szEquals ) )
					++szEquals;
				strncpy( lpReturnedString, szEquals, nSize );
            nReturn = strlen( lpReturnedString );
				}
			}
		fclose( fp );
		}
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
	char szTempName[ _MAX_PATH ];
	char szBuff[BUF_LEN];

	if (( fp = fopen( lpFileName, "a+t" ) )!=0)
		{
		tmpnam( szTempName );
		fpo = fopen( szTempName, "w+t" );

		if ( !FindApp( fp, lpApplicationName, fpo ) )
			{ // Create a new application field
			fseek( fpo, 0, SEEK_END );
			fprintf( fpo, "\n[%s]\n", lpApplicationName );
			fprintf( fpo, "%s = %s\n", lpKeyName, lpString );
			}
		else
			{
			rewind( fpo );
			FindLine( fp, lpKeyName, lpApplicationName, szBuff, BUF_LEN, fpo );

			// Add replacement (or new) line
			strncpy( szBuff, lpString, 255 );
//                        unstrip_ws( szBuff );
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
// '_' misstake?		_unlink( lpFileName );
// get out of my way, pigsys
#undef rename
		rename( szTempName, lpFileName );
		}

	return 1;
}


bool
WritePrivateProfileInt( const char *lpApplicationName, const char *lpKeyName, int nValue, const char *lpFileName )
{
	char szBuff[BUF_LEN];

	ftell(NULL);
	sprintf( szBuff, "%d", nValue );
	return WritePrivateProfileString( lpApplicationName, lpKeyName, szBuff, lpFileName );
}


#if 0

// Need to [re]develop later [if needed]

/*const*/ char szSystemIni[] = "e:\\dr\\bin";

int
GetProfileInt( char* lpszSection, char* lpszEntry, int nDefault )
{
	return GetPrivateProfileInt( lpszSection, lpszEntry, nDefault, szSystemIni );
}


int
GetProfileString( char *lpApplicationName, char *lpKeyName, char *lpDefault, char *lpReturnedString, int nSize )
{
	return GetPrivateProfileString( lpApplicationName, lpKeyName, lpDefault, lpReturnedString, nSize, szSystemIni );
}


bool
WriteProfileString( char *lpApplicationName, char *lpKeyName, char *lpString )
{
	return WritePrivateProfileString( lpApplicationName, lpKeyName, lpString, szSystemIni );
}


bool
WriteProfileInt( char *lpApplicationName, char *lpKeyName, int nValue )
{
	return WritePrivateProfileInt( lpApplicationName, lpKeyName, nValue, szSystemIni );
}

#endif

// class.c

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "exprtn.h"
#include "../types.h"
#include "../util.h"

char*
FindClass( const char* name )
	{
	static char classInfoID[] = "Cave Logic Studios Class Object Editor v0.1";
	char* szClassName = NULL;

	Header _far* fentry;
	pxp_get_appdata_chunk( (char*)name, classInfoID, fentry );

    if ( FP_OFF( fentry ) )			// If there was a chunk returned
    	{
		Header _far* appname = fentry + 1;
		Header _far* string  = (Header _far*)( ((char _far*)appname) + appname->length );

		ClassChunk _far* infoChunk = (ClassChunk _far*)(string + 1);

		const char* oadPath = GetOadDir();
		szClassName = (char*)malloc( string->length + strlen( oadPath ) + 1 );
		assert( szClassName );

		strcpy( szClassName, oadPath );
		fnstrcpy( szClassName+strlen( szClassName ), infoChunk->oadFilename );
		*( szClassName + string->length + strlen( oadPath ) ) = '\0';
    	}

	return szClassName;
	}

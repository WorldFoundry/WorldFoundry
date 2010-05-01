// loaddll.cpp

#include <max.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>

HINSTANCE
LoadMaxLibrary( const char* szMaxPlugIn )
{
	HINSTANCE hInstance = 0;
	assert( szMaxPlugIn );
	assert( *szMaxPlugIn );

	Interface* ip = GetCOREInterface();
	assert( ip );

	for ( int idxPlugIn=0; !hInstance && idxPlugIn < ip->GetPlugInEntryCount(); ++idxPlugIn )
	{
		char szDllName[ _MAX_PATH ];
		sprintf( szDllName, "%s/%s", ip->GetPlugInDir( idxPlugIn ), szMaxPlugIn );
		hInstance = LoadLibrary( szDllName );
	}

	return hInstance;
}

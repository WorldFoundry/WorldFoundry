// main.cpp
// Copyright 1997 Recombinant Limited.  All Rights Reserved.
// by William B. Norris IV

#include "attrib.h"

HINSTANCE hInstance;

/** public functions **/
BOOL WINAPI 
DllMain( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	if ( !hInstance )
	{
		hInstance = hinstDLL;
		InitCustomControls(hInstance);
		InitCommonControls();
	}
			
	return TRUE;
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

/*__declspec( dllexport )*/ const TCHAR * 
LibDescription() 
{
	return _T("World Foundry(TM) Attribute Editor v1.0.3"); 
}

/*__declspec( dllexport )*/ int LibNumberClasses() 
{
	return 1;
}

/*__declspec( dllexport )*/ ClassDesc* 
LibClassDesc( int i ) 
{
	assert( i == 0 );
	return GetPropertiesDesc();
}

// Return version so can detect obsolete DLLs
/*__declspec( dllexport )*/ ULONG 
LibVersion() 
{ 
	return VERSION_3DSMAX; 
}


TCHAR*
GetString(int id)
{
	static TCHAR buf[256];

	if ( hInstance )
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}


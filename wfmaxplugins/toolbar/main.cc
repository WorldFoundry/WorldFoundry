// main.cpp

#include "toolbar.h"

HINSTANCE hInstance;

BOOL WINAPI
DllMain( HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved ) 
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

__declspec( dllexport ) const TCHAR *
LibDescription() { return _T("World Foundry Toolbar"); }

/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int LibNumberClasses() 
{
	return 1;
}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) 
{
	switch(i) 
	{
		case 0: return GetPropertiesDesc();
		default: return 0;
	}
}

// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() 
{ 
	return VERSION_3DSMAX; 
}


TCHAR* GetString( int id )
{
	static TCHAR buf[256];

	if ( hInstance )
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


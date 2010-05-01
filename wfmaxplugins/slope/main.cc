// main.cpp

#include "prim.h"

HINSTANCE hInstance;

// public functions 
BOOL WINAPI
DllMain( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	if ( !hInstance )
	{
		hInstance = hinstDLL;
		InitCustomControls( hInstance );
		InitCommonControls();
	}

	switch(fdwReason) 
	{
		case DLL_PROCESS_ATTACH:
			//MessageBox(NULL,_T("PRIM.DLL: DllMain)",_T("Prim"),MB_OK);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
	return _T(" World Foundry Sloped Surface Primitive "); 
}


/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int 
LibNumberClasses() 
{
	return 1;
}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i) 
{
	assert( i == 0 );
	return GetBoxobjDesc();
}


// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG 
LibVersion() 
{ 
	return VERSION_3DSMAX; 
}


TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;

	return NULL;
}

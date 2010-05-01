//=============================================================================
// standard.cpp: an attempt to put all non-changing code here
// for a simple object primitive
// Kevin T. Seghetti
// included by <objectname>.cpp
//=============================================================================

//=============================================================================
// This method returns the dimension of the parameter requested.
// This dimension describes the type and magnitude of the value
// stored by the parameter.

ParamDimension*
CycloneTerrainObject::GetParameterDim(int pbIndex)
{
	assert(pbIndex < PARAMETER_ENTRIES);
	return cycloneTerrainParameterEntryArray[pbIndex]._dimension;
}

//=============================================================================
// This method returns the name of the parameter requested.

TSTR
CycloneTerrainObject::GetParameterName(int pbIndex)
{
	assert(pbIndex < PARAMETER_ENTRIES);
	return TSTR(cycloneTerrainParameterEntryArray[pbIndex]._name);
}

//=============================================================================
// This function returns a pointer to a string in the string table of
// the resource library.  Note that this function maintains the buffer
// and that only one string is loaded at a time.  Therefore if you intend
// to use this string, you must copy to another buffer since it will
// be overwritten on the next GetString() call.
//=============================================================================

TCHAR*
GetString(int id)
{
	static TCHAR buf[256];
	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


// The following five functions are used by every plug-in DLL.
//=============================================================================
// | The DLL and Library Functions
//=============================================================================
// This function is called by Windows when the DLL is loaded.  This
// function may also be called many times during time critical operations
// like rendering.  Therefore developers need to be careful what they
// do inside this function.  In the code below, note how after the DLL is
// loaded the first time only a few statements are executed.

//=============================================================================
// This is the DLL instance handle passed in when the plug-in is
// loaded at startup.

HINSTANCE hInstance = NULL;

//=============================================================================

BOOL WINAPI
DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	if(!hInstance)
	{
		hInstance = hinstDLL;			// Hang on to this DLL's instance handle.
		InitCustomControls(hInstance);		// Initialize MAX's custom controls
		InitCommonControls();               // Initialize Win95 controls
	}
	assert(hInstance == hinstDLL);
	return(TRUE);
}

//=============================================================================
// This function returns the number of plug-in classes this DLL implements

__declspec( dllexport ) int
LibNumberClasses()
{
	return 1;
}

//=============================================================================
// This function return the ith class descriptor

__declspec( dllexport ) ClassDesc*
LibClassDesc(int i)
{
	switch(i)
	{
		case 0:
			return GetCycloneTerrainDesc();
		default:
			return 0;
	}
}


//=============================================================================
// This function returns a string that describes the DLL.  This string appears in
// the File / Summary Info / Plug-In Info dialog box.

__declspec( dllexport ) const TCHAR*
LibDescription()
{
	return GetString(IDS_LIB_DESC);
}

//=============================================================================
// This function returns a pre-defined constant indicating the version of
// the system under which it was compiled.  It is used to allow the system
// to catch obsolete DLLs.

__declspec( dllexport ) ULONG
LibVersion()
{
	return VERSION_3DSMAX;
}

//=============================================================================

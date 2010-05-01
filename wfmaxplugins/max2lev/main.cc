///////////////////////////////////////////////////////////////////////////////
//																			 //
// main.cc	World Foundry
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "global.hpp"
#include "max2lev.hpp"	// C++ header file
#include "max2lev.h"		// Resource header file
#include "../lib/registry.h"		// registry reader
#include <oas/oad.h>
//#include <iffwrite/iffwrite.h>
#include "box.h"

static LVLExportClassDesc LVLExportCD;		// static instance of the export class descriptor
HINSTANCE hInstance;                        // this DLL's instance handle (some Windows thing)
Interface* gMaxInterface;					// Global pointer to MAX interface class

///////////////////////////////////////////////////////////////////////////////
// Functions called by MAX when our DLL is loaded

BOOL WINAPI
DllMain( HINSTANCE hinstDLL, ULONG fdwReason, LPVOID lpvReserved )
{
	if ( !hInstance )
	{
		hInstance = hinstDLL;
		InitCustomControls( hInstance );
		InitCommonControls();
	}

	return TRUE;
}


__declspec( dllexport ) int
LibNumberClasses()
{
	return 1;
}


__declspec( dllexport ) ClassDesc*
LibClassDesc( int i )
{
	assert( i == 0 );
	return &LVLExportCD;
}


__declspec( dllexport ) const TCHAR*
LibDescription()
{
	return _T( "World Foundry IFF Level Exporter v" LEVELCON_VER );
}


__declspec( dllexport ) ULONG
LibVersion()
{
	return VERSION_3DSMAX;
}

///////////////////////////////////////////////////////////////////////////////
// Miscelaneous support functions (GUI crap, etc.)

TCHAR*
GetString( int id )
{
	static TCHAR buf[256];
	if ( hInstance )
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}


static void
MessageBox(int s1, int s2)
{
	TSTR str1( GetString( s1 ) );
	TSTR str2( GetString( s2 ) );
	MessageBox( GetActiveWindow(), str1.data(), str2.data(), MB_OK );
}


static int
MessageBox(int s1, int s2, int option = MB_OK)
{
	TSTR str1(GetString(s1));
	TSTR str2(GetString(s2));
	return MessageBox(GetActiveWindow(), str1, str2, option);
}


static int
Alert(int s1, int s2 /*= IDS_LVLEXP*/, int option = MB_OK)
{
	return (int)MessageBox(s1, s2, option);
}

///////////////////////////////////////////////////////////////////////////////
// Actual methods of the LVLExport class

LVLExport::LVLExport()
{
}


LVLExport::~LVLExport()
{
}


int
LVLExport::ExtCount()
{
	return 1;
}


const TCHAR*
LVLExport::Ext(int n)		// Extensions supported for import/export modules
{
	switch ( n )
	{
		case 0:
			return _T("lev");

		case 1:
			return _T("iffbin");

		default:
			assert( 0 );
	}

	return _T( "" );	// to shut up compiler
}


const TCHAR*
LVLExport::LongDesc()		// Long ASCII description (i.e. "Targa 2.0 Image File")
{
	return ShortDesc();
}


const TCHAR*
LVLExport::ShortDesc()		// Short ASCII description (i.e. "Targa")
{
	return _T("World Foundry Level IFF File");
}


const TCHAR*
LVLExport::AuthorName()		// ASCII Author name
{
	return _T("William B. Norris IV");
}


const TCHAR*
LVLExport::CopyrightMessage() 	// ASCII Copyright message
{
	return _T("Copyright 1997-2000 World Foundry Group.  All Rights Reserved.");
}


const TCHAR*
LVLExport::OtherMessage1()		// Other message #1
{
	return _T("OtherMessage1");
}


const TCHAR*
LVLExport::OtherMessage2()		// Other message #2
{
	return _T("OtherMessage2");
}


unsigned int
LVLExport::Version()			// Version number * 100 (i.e. v3.01 = 301)
{
	return VERSION_3DSMAX;
}

void AboutBox( HWND hDlg );

void
LVLExport::ShowAbout( HWND hWnd )
{
	assert( hWnd );
	AboutBox( hWnd );
}


extern "C" bool max2ifflvl_Query( ostream& s, SceneEnumProc* theSceneEnum );

int
#if MAX_RELEASE < 2000 
LVLExport::DoExport( const TCHAR* name, ExpInterface* ei, Interface* gi )
#else
LVLExport::DoExport( const TCHAR* name, ExpInterface* ei, Interface* gi, int )
#endif
{
	assert( gi );
	gMaxInterface = gi;

	// Ask the scene to enumerate all of its nodes so we can determine if there are any we can use
	SceneEnumProc myScene( ei->theScene, gi->GetTime(), gi );

	try
	{
		ofstream fp( name, ios::out | ios::binary );
		max2ifflvl_Query( fp, &myScene );
	}
	catch ( LVLExporterException theException )
	{
		return 0;	// Return to MAX with failure
	}

	return 1;		// Return to MAX with success

}

//==============================================================================

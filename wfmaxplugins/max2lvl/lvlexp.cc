///////////////////////////////////////////////////////////////////////////////
//																			 //
// lvlexp.cpp	LVL Exporter plugin for 3DS Max								 //
//																			 //
//	(This is the new World Foundry replacement for LEVELCON.EXE)			 //
//	Copyright 1996,1997 by Recombinant Ltd.  All rights reserved.			 //
//																			 //
//	Written 1/6/97 by Phil Torre											 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "global.hpp"
#include "lvlexp.hpp"	// C++ header file
#include "lvlexp.h"		// Resource header file
#include "../lib/registry.h"		// registry reader

static LVLExportClassDesc LVLExportCD;		// static instance of the export class descriptor
HINSTANCE hInstance;                        // this DLL's instance handle (some Windows thing)

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
	if ( i == 0 )
		return &LVLExportCD;
	else
		return 0;
}


__declspec( dllexport ) const TCHAR*
LibDescription()
{
	return _T( "World Foundry LVL Exporter v" LEVELCON_VER );
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
Alert(int s1, int s2 = IDS_LVLEXP, int option = MB_OK)
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


const TCHAR *
LVLExport::Ext(int n)		// Extensions supported for import/export modules
{
	switch(n)
	{
		case 0:
			return _T("lvl");
	}
	return _T("");
}


const TCHAR *
LVLExport::LongDesc()		// Long ASCII description (i.e. "Targa 2.0 Image File")
{
	return ShortDesc();
}


const TCHAR *
LVLExport::ShortDesc()		// Short ASCII description (i.e. "Targa")
{
	return _T("World Foundry Level File");
}


const TCHAR *
LVLExport::AuthorName()		// ASCII Author name
{
	return _T("Kevin T. Seghetti and Phil Torre");
}


const TCHAR *
LVLExport::CopyrightMessage() 	// ASCII Copyright message
{
	return _T("Copyright 1997,98,99 World Foundry Group.");
}


const TCHAR *
LVLExport::OtherMessage1()		// Other message #1
{
	return _T("OtherMessage1");
}


const TCHAR *
LVLExport::OtherMessage2()		// Other message #2
{
	return _T("OtherMessage2");
}


unsigned int
LVLExport::Version()			// Version number * 100 (i.e. v3.01 = 301)
{
	return 0;
}

void AboutBox( HWND hDlg );

void
LVLExport::ShowAbout( HWND hWnd )
{
	assert( hWnd );
	AboutBox( hWnd );
}

extern int LevelconMain( int argc, char *argv[] );

//==============================================================================

inline
char* EatWhiteSpace(char* string)
{
	assert(string);
	while(*string && (*string == ' ' || *string == 0xa || *string == 0xd))
	 {
		if(*string == 0xa || *string == 0xd)
			*string = ' ';
		string++;
	 }
	return(string);
}

//==============================================================================

Interface* gMaxInterface;	// Global pointer to MAX interface class

//==============================================================================

char* LoadTextFile( const char* _szFilename, int& nSize );
char* pszWarnings;
int cbWarnings;

BOOL CALLBACK
ShowWarningsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_INITDIALOG:
		{
			HWND _hwndList = GetDlgItem( hDlg, IDC_TEXT );
			assert( _hwndList );

			char* pEnd = pszWarnings + cbWarnings;
			char* p = pszWarnings;
			while ( p < pEnd )
			{
				char* pCrLf = strchr( p, '\n' );
				if ( pCrLf )
					*pCrLf = '\0';

				int err = ListBox_AddString( _hwndList, p );
				assert( err != CB_ERR );

				p = pCrLf ? pCrLf + 1 : pEnd;
			}

			return TRUE;
		}

		case WM_COMMAND:
		{
			UINT button= LOWORD( wParam );
			if ( button == IDOK || button == IDCANCEL )
			{
				HWND _hwndList = GetDlgItem( hDlg, IDC_TEXT );
				assert( _hwndList );
				ListBox_ResetContent( _hwndList );
				EndDialog( hDlg, LOWORD( wParam ) == IDOK );
			}
			return TRUE;
		}

		default:
			return FALSE;
	}
}

void
LVLExport::ShowWarnings()
{
	pszWarnings = (char*)LoadTextFile( "c:\\levelcon.warn.txt", cbWarnings );
	if ( pszWarnings && ( cbWarnings > 2 ) )
		DialogBox( hInstance, MAKEINTRESOURCE( IDD_DIALOGSTREAM ), GetActiveWindow(), (DLGPROC)ShowWarningsDlgProc );
	if ( pszWarnings )
		free( pszWarnings ), pszWarnings = NULL;
}


int
#if MAX_RELEASE < 2000
LVLExport::DoExport( const TCHAR* name, ExpInterface*, Interface* gi )
#else
LVLExport::DoExport( const TCHAR* name, ExpInterface*, Interface* gi, int )
#endif
{
	assert( gi );
	gMaxInterface = gi;

	INode* _pRoot = gi->GetRootNode();
	assert( _pRoot );
	int numObjects = _pRoot->NumberOfChildren();

	// If no nodes are usable, bail out
	if(numObjects == 0)
	{
		MessageBox( gi->GetMAXHWnd(), "I see no objects here.", "LVL Exporter", MB_OK );
		return 0;
	}

	// kts new (but not very good) command line handler
	// kts command line disection
	const int MAX_ARGC = 30;
	char* argvPtrs[MAX_ARGC];

	argvPtrs[0] = "levelcon.exe";					// set argv[0] to program name
	int argCount = 1;

	const int MAX_CMD_LINE = 200;
	char optionsBuffer[MAX_CMD_LINE];
	string lc_path;

	if ( GetLocalMachineStringRegistryEntry("Software\\World Foundry\\GDK\\max2lvl","OPTIONS",optionsBuffer,MAX_CMD_LINE) )
	{
		char* pLine = &optionsBuffer[0];
		while(*pLine )		// parse command line into arguments
		{
			pLine = EatWhiteSpace(pLine);
			argvPtrs[argCount] = pLine;
			while(*pLine && *pLine != ' ' && *pLine != 0xa && *pLine != 0xd)
			{
				assert(pLine < (&optionsBuffer[0] + MAX_CMD_LINE));			// insure we don't walk off end of file
				pLine++;
			}
			if(*pLine)						// if not at end of inputBuffer, write a zero, then keep going
				*pLine++ = 0;
			pLine = EatWhiteSpace(pLine);

			assert(argCount < MAX_ARGC);
			argCount++;
		}
	}

	// now add manditory entries
	argvPtrs[argCount++] = "-n";
	argvPtrs[argCount++] = gi->GetCurFilePath().data();	// Used to find the default scripts directory
	char OADdir[_MAX_PATH];
	if (!GetLocalMachineStringRegistryEntry("Software\\World Foundry\\GDK","OAD_DIR",OADdir,_MAX_PATH))
	{
		MessageBox(gi->GetMAXHWnd(), "Cannot find Registry entry OAD_DIR. \nPlease see the installation instructions", "LVL Exporter", MB_OK);
		return (0);
	}
	else
	{
		lc_path = string( OADdir );
		lc_path += "/objects.lc";
		argvPtrs[argCount++] = (char*)lc_path.c_str();
		argvPtrs[argCount++] = (char*)name;
	}

	HCURSOR hcursorCurrent = GetCursor();
	assert( hcursorCurrent );
	HCURSOR hcursorWait = LoadCursor( NULL, IDC_WAIT );
	assert( hcursorWait );
	SetCursor( hcursorWait );

	try
	{
		// Call the Levelcon code
		LevelconMain( argCount, argvPtrs );
	}
	catch ( LVLExporterException theException )
	{
		ShowWarnings();
		SetCursor( hcursorCurrent );
		return 0;	// Return to MAX with failure
	}

	ShowWarnings();
	SetCursor( hcursorCurrent );
	gi->ForceCompleteRedraw();
	return 1;		// Return to MAX with success

}

//==============================================================================

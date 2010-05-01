// main.cpp for "select.dlu"

#include "select.h"

HINSTANCE hInstance;

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


//------------------------------------------------------
// This is the interface to Jaguar:
//------------------------------------------------------

__declspec( dllexport ) const TCHAR*
LibDescription()
{
	return _T("World Foundry(TM) Object Selector");
}


/// MUST CHANGE THIS NUMBER WHEN ADD NEW CLASS
__declspec( dllexport ) int
LibNumberClasses()
{
	return 0;
}


__declspec( dllexport ) ClassDesc*
LibClassDesc(int i)
{
	return NULL;
}


// Return version so can detect obsolete DLLs
__declspec( dllexport ) ULONG
LibVersion()
{
	return VERSION_3DSMAX;
}




////////////////////////////////////////////////////////////////////////////////

BOOL CALLBACK
MaxWndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( msg == WM_INITMENUPOPUP )
	{
		assert( theSelect.ip );
		INode* pRoot = theSelect.ip->GetRootNode();
		assert( pRoot );
		EnableMenuItem( (HMENU)wParam, Select::SELECT_WORLD_FOUNDRY_MENUITEM, pRoot->NumberOfChildren() == 0 ? MF_GRAYED : MF_ENABLED );
	}
	else if ( ( msg == WM_COMMAND )
	&& ( LOWORD( wParam ) == Select::SELECT_WORLD_FOUNDRY_MENUITEM )
	)
	{
		theSelect.DoDialog();
		return TRUE;
	}

	assert( theSelect.wndProc );
	return theSelect.wndProc( hWnd, msg, wParam, lParam );
}

////////////////////////////////////////////////////////////////////////////////

void
WaitForMaxWindow()
{
	assert( theSelect.ip );
	HWND hwnd;
	do
	{
		hwnd = theSelect.ip->GetMAXHWnd();
		Sleep( 100 );
	}
	while ( !hwnd );

	assert( hwnd );
	HMENU hMaxMenu = GetMenu( hwnd );
	assert( hMaxMenu );
	HMENU hEditMenu = GetSubMenu( hMaxMenu, 1 );		// Edit
	assert( hEditMenu );
	HMENU hSelectMenu = GetSubMenu( hEditMenu, 12 );	// Select by ->
	assert( hSelectMenu );

	AppendMenu( hSelectMenu, MF_SEPARATOR, 0, NULL );
	AppendMenu( hSelectMenu, MF_STRING, Select::SELECT_WORLD_FOUNDRY_MENUITEM, "World Foundry &Attributes..." );

	theSelect.wndProc = (WNDPROC)GetWindowLong( hwnd, GWL_WNDPROC );
	assert( theSelect.wndProc );
	SubclassWindow( hwnd, MaxWndProc );

	assert( hInstance );
	HMENU hmenu = LoadMenu( hInstance, MAKEINTRESOURCE( IDR_HEADER_RMENU ) );
	assert( hmenu );
	theSelect._columnRCmenu = GetSubMenu( hmenu, 0 );
	assert( theSelect._columnRCmenu );
}


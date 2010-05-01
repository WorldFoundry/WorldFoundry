// about.cpp

#include "global.hpp"
//#include "lvlexp.hpp"
//#include <windows.h>
//#include <windowsx.h>
//#include <assert.h>
#include "lvlexp.h"
extern HINSTANCE hInstance;

BOOL CALLBACK
AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	if ( msg == WM_COMMAND )
	{
		EndDialog( hDlg, LOWORD( wParam ) == IDOK );
		return TRUE;
	}
	else
		return FALSE;
}


void
AboutBox( HWND hDlg )
{
	assert( hInstance );
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_ABOUT ), hDlg, (DLGPROC)AboutDlgProc );
}

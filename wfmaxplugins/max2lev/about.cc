// about.cpp

#include "global.hpp"
#include "max2lev.h"
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

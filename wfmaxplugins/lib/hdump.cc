// hudmp.cpp

#include "Max.h"
#include "utilapi.h"
#include <windowsx.h>
#include <stdio.h>
#include <assert.h>
//#include "../resource.h"

static HINSTANCE hInstance;
static unsigned char* data;
static size_t size;

static BOOL CALLBACK 
hdump_Proc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDOK:
					EndDialog( hDlg, 1 );
					break;

				case IDCANCEL:
					EndDialog( hDlg, 0 );
					break;

				default:
					return FALSE;
			}
		}

		default:
			return FALSE;
	}
	return TRUE; 
}


int
hdump( HINSTANCE hInstance, HWND hwnd, void* data, int size )
{
	assert( hInstance );
	assert( data );
	assert( size > 0 );
	::hInstance = hInstance;
	::data = (unsigned char*)data;
	::size = size;
	return DialogBox( hInstance, "HDUMP", hwnd, (DLGPROC)hdump_Proc );
}

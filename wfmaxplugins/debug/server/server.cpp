// server.cpp

#include <windows.h>
#include <ddeml.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <commctrl.h>
#include <windowsx.h>
#include "watch.h"
#include "../resource.h"

enum {
	WM_USER_INITIATE = WM_USER + 1,
	WM_TRAYCALLBACK
	};

const ID_TIMER = 1;

LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
HDDEDATA CALLBACK DdeCallback( UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD );
BOOL CALLBACK AboutDlgProc( HWND, UINT, WPARAM, LPARAM );

char szAppName[] = "WfDbgSrv";
char szTopic[] = "Watch";
DWORD idInst;
HINSTANCE hInstance;
HWND hwnd;
NOTIFYICONDATA tnd;

std::vector< Watch* > _watches;

int WINAPI
WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow )
{
	MSG msg;
	WNDCLASSEX wndclass;

	::hInstance = hInstance;

	wndclass.cbSize = sizeof( wndclass );
	wndclass.style = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	assert( hInstance );
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon( hInstance, szAppName );
	assert( wndclass.hIcon );
	wndclass.hCursor = LoadCursor( NULL, IDC_ARROW );
	assert( wndclass.hCursor );
	wndclass.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	assert( wndclass.hbrBackground );
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	assert( wndclass.lpszClassName );
	wndclass.hIconSm = LoadIcon( hInstance, szAppName );
	assert( wndclass.hIconSm );
	RegisterClassEx( &wndclass );

	hwnd = CreateWindow( szAppName, "World Foundry Debugger [Server Placeholder]",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL );
	assert( hwnd );
//	ShowWindow( hwnd, SW_SHOWMINNOACTIVE );
	UpdateWindow( hwnd );

	{ // Small icon in tray
	strcpy( tnd.szTip, "World Foundry Debugger [Server Placeholder]" );
	tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	tnd.uID	= (UINT)IDI_WINLOGO;
	tnd.cbSize = sizeof( tnd );
	tnd.hWnd = hwnd;
	tnd.hIcon = wndclass.hIconSm;
	tnd.uCallbackMessage = WM_TRAYCALLBACK;

	Shell_NotifyIcon( NIM_ADD, &tnd );
	}
	
	// Initialize for using DDEML
	if ( DdeInitialize( &idInst, (PFNCALLBACK)&DdeCallback,
		CBF_FAIL_EXECUTES | CBF_FAIL_POKES |
		CBF_SKIP_REGISTRATIONS | CBF_SKIP_UNREGISTRATIONS, 0 ) )
	{
		MessageBox( hwnd, "Could not initialize server!",
			szAppName, MB_ICONEXCLAMATION | MB_OK );
		DestroyWindow( hwnd );
		return FALSE;
	}

	SetTimer( hwnd, ID_TIMER, 2000, NULL );
	SendMessage( hwnd, WM_USER_INITIATE, 0, 0L );

	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	Shell_NotifyIcon( NIM_DELETE, &tnd );

	DdeUninitialize( idInst );
	KillTimer( hwnd, ID_TIMER );

	return msg.wParam;
}


HDDEDATA CALLBACK
DdeCallback( UINT iType, UINT iFmt, HCONV hConv, HSZ hsz1, HSZ hsz2, 
	HDDEDATA hData, DWORD dwData1, DWORD dwData2 )
{
	char szBuffer[ 128 ];

	switch ( iType )
	{
		case XTYP_CONNECT:
		{
			DdeQueryString( idInst, hsz2, szBuffer, sizeof( szBuffer ), 0 );
			if ( strcmp( szBuffer, szAppName ) != 0 )
				return (HDDEDATA)FALSE;

			DdeQueryString( idInst, hsz1, szBuffer, sizeof( szBuffer ), 0 );
			if ( strcmp( szBuffer, szTopic ) != 0 )
				return (HDDEDATA)FALSE;

			return (HDDEDATA)TRUE;
		}

		case XTYP_ADVSTART:
		{
			if ( iFmt != CF_TEXT )
				return (HDDEDATA)FALSE;

			char szWatchName[ 256 ];
			DdeQueryString( idInst, hsz2, szWatchName, sizeof( szWatchName ), 0 );

			// check for existing

			Watch* pWatch = new Watch( szWatchName );
			assert( pWatch );
			_watches.push_back( pWatch );

			PostMessage( hwnd, WM_TIMER, 0, 0L );
			
			return (HDDEDATA)TRUE;
		}

		case XTYP_REQUEST:
		case XTYP_ADVREQ:
		{
			if ( iFmt != CF_TEXT )
				return (HDDEDATA)FALSE;

			char szWatchName[ 256 ];
			DdeQueryString( idInst, hsz2, szWatchName, sizeof( szWatchName ), 0 );

			std::vector< Watch* >::iterator iWatch;
			for ( iWatch = _watch&$"$+$$&,*)+)( "#(&# #'%!###$&%!!&$')'*1'&%#$"$%%$#! "#%&## "$"!!#$''%##&($$"""$$#"$"&) &$"%$#!"  !"!" 
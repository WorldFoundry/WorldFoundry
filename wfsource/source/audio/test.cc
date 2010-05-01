// test.cc

// !!! This test program is currently for Windows only
// !!! [in fact, Windows is the only version currently implemented]

#if defined(__WIN__)
#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>			// why is this needed first? problems with including our stuff first!
#endif
#include <audio/audio.hp>

#include <cassert>
#include <iostream>
//using namespace std;
#include <pigsys/pigsys.hp>
#include <hal/hal.h>
#include <gfx/display.hp>
#include <loadfile/loadfile.hp>

////////////////////////////////////////////////////////////////////////////////

const char szAppName[] = "audio test";

//void* LoadBinaryFile( const char* szFilename, unsigned long &sizeOfFile );

const char* szFilenames[ 3 ] = {
	"psychic.wav",
	"goede morgen.wav",
	"drums.wav",
};

////////////////////////////////////////////////////////////////////////////////
#if defined(__WIN__)
extern HWND hWnd;
#endif

void
PIGSMain( int argc, char* argv[] )
//test( HWND hwnd )
{
{
	Display display(10,0,0,100,100,HALLmalloc);

	SoundDevice soundHw;

	int32 cbWave;
	int8* wave = (int8*)LoadBinaryFile( argc>1 ? argv[1] : "drums.wav", cbWave );
	assert( wave );

	binistream binis( wave, cbWave );
	SoundBuffer* buffer = soundHw.CreateSoundBuffer( binis );
//	SoundBuffer* buffer = soundHw.CreateSoundBuffer( wave, cbWave );
	assert( ValidPtr( buffer ) );

	buffer->play();

#if defined(__WIN__)
	Sleep( 3500 );
#endif

	delete buffer;
	}

PIGSExit();
}


#if 0

long FAR PASCAL
WndProc( HWND _hwnd, UINT message, UINT wParam, LONG lParam )
{
	PAINTSTRUCT ps;
	RECT rect;

	switch ( message )
	{
#if 0
		case WM_ACTIVATEAPP:
			bActive = bool( wParam );
			return 0;
#endif

		case WM_PAINT:
		{
			HWND hwnd = _hwnd;
			HDC hdc = BeginPaint( hwnd, &ps );
			GetClientRect( hwnd, &rect );
			test( hwnd );
			EndPaint( hwnd, &ps );
			// fall through to quit
		}

		case WM_DESTROY:
			//GameShutdown();
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( _hwnd, message, wParam, lParam );
}



int PASCAL
WinMain( HANDLE hInstance, HANDLE hPrevInstance, LPSTR /*lpszCmdParam*/, int nCmdShow )
{
	cout << "Audio test program" << endl;

	if( !hPrevInstance )
	{
		WNDCLASS wndclass;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = WndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
		wndclass.hCursor = LoadCursor( NULL, IDC_ARROW );
		wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szAppName;
		RegisterClass( &wndclass );
	}

	ShowCursor( FALSE );

//	assert( _halWindowWidth > 0 );
//	assert( _halWindowHeight > 0 );
	HWND hwnd = CreateWindowEx(
		0,
		szAppName,
		szAppName,
		WS_OVERLAPPEDWINDOW,
		0, 0, 320+8, 240+27,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	ShowWindow( hwnd, nCmdShow );
	UpdateWindow( hwnd );
	SetFocus( hwnd );

	MSG msg;
	for( ;; )
	{
		if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if ( !GetMessage( &msg, NULL, 0, 0 ) )
				return msg.wParam;
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
#if 0
		else if ( bActive )
		{
			//DrawScreen();
			//GameLoop();
		}
#endif
		else
			WaitMessage();
	}
	//AssertMsg( 0, "End of HAL" );

	//NOTREACHED
	return msg.wParam;
}

#endif

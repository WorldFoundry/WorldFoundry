// test.cc

#include <cassert>
#include <iostream>
using namespace std;

#include <dsound.h>

LPDIRECTSOUND lpDirectSound;
const char szAppName[] = "audio test";


const NUM_SOUNDS = 3;


void* LoadBinaryFile( const char* szFilename, unsigned long &sizeOfFile );

const char* szFilenames[ 3 ] = {
	"psychic.wav",
	"goede morgen.wav",
	"drums.wav",
};


void
test( HWND hwnd )
{
	HRESULT hr;

	hr = DirectSoundCreate( NULL, &lpDirectSound, NULL );
	assert( hr == DS_OK );
   	lpDirectSound->SetCooperativeLevel( hwnd, DSSCL_PRIORITY );

    LPDIRECTSOUNDBUFFER lplpDsb[ NUM_SOUNDS ];
	for ( int i=0; i<NUM_SOUNDS; ++i )
		lplpDsb[ i ] = NULL;

    PCMWAVEFORMAT pcmwf;
    DSBUFFERDESC dsbdesc;

    // Set up wave format structure.
    memset( &pcmwf, 0, sizeof( pcmwf ) );
    pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
//    pcmwf.wf.wFormatTag = WAVE_FORMAT_2M16;
    pcmwf.wf.nChannels = 2;
    pcmwf.wf.nSamplesPerSec = 22050;
    pcmwf.wf.nBlockAlign = 4;
    pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample = 16;

    // Set up DSBUFFERDESC structure.
    memset( &dsbdesc, 0, sizeof( dsbdesc ) ); // Zero it out.
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
//    dsbdesc.dwBufferBytes = 3 * pcmwf.wf.nAvgBytesPerSec;		// 3-second buffer
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

	LPBYTE lpbSoundData[ NUM_SOUNDS ];

    // Create buffer.
	for ( i=0; i<NUM_SOUNDS; ++i )
	{
		lpbSoundData[ i ] = (LPBYTE)LoadBinaryFile( szFilenames[ i ], dsbdesc.dwBufferBytes );
		assert( lpbSoundData[ i ] );

    	hr = lpDirectSound->CreateSoundBuffer( &dsbdesc, &lplpDsb[ i ], NULL );
		assert( hr == DS_OK );

#if 0
		lpbSoundData[ i ] = (LPBYTE)malloc( dsbdesc.dwBufferBytes );
		assert( lpbSoundData[ i ] );
		for ( int b=0; b<dsbdesc.dwBufferBytes; ++b )
		{
			*( lpbSoundData[ i ] + b ) = b * i;
			*( lpbSoundData[ i ] + b ) = b * ( 128 / (i+1) );
		}
#endif

		//AppWriteDataToBuffer( LPDIRECTSOUNDBUFFER lpDsb, DWORD dwOffset, LPBYTE lpbSoundData, DWORD dwSoundBytes )
		LPVOID lpvPtr1;
		DWORD dwBytes1;
    	LPVOID lpvPtr2;
		DWORD dwBytes2;

		// Obtain write pointer.
	    hr = lplpDsb[ i ]->Lock( 0/*dwOffset*/, dsbdesc.dwBufferBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0 );

#if 0
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
    if ( hr == DSERR_BUFFERLOST )
	{
		lplpDsb->Restore();
        hr = lplpDsb->Lock( 0, dsbdesc.dwBufferBytes, &lpvPtr1, &dwAudio1, &lpvPtr2, &dwAudio2, 0 );
	}
#endif

		assert( hr == DS_OK );

		// Write to pointers.
    	CopyMemory( lpvPtr1, lpbSoundData[ i ], dwBytes1 );
    	if ( lpvPtr2 )
        	CopyMemory( lpvPtr2, lpbSoundData[ i ]+dwBytes1, dwBytes2 );

    	// Release the data back to DirectSound.
    	hr = lplpDsb[ i ]->Unlock( lpvPtr1, dwBytes1, lpvPtr2, dwBytes2 );
		assert( hr == DS_OK );
	}

	for ( i=0; i<NUM_SOUNDS; ++i )
	{
		hr = lplpDsb[ i ]->Play( 0, 0, 0 );
		assert( hr == DS_OK );
	}

	Sleep( 3500 );

	for ( i=0; i<NUM_SOUNDS; ++i )
	{
		hr = lplpDsb[ i ]->Stop();
		assert( hr == DS_OK );

		hr = lplpDsb[ i ]->Release();
		assert( hr == DS_OK );

		assert( lpbSoundData[ i ] );
		free( lpbSoundData[ i ] );
	}

   	hr = lpDirectSound->Release();
	assert( hr == DS_OK );
}




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

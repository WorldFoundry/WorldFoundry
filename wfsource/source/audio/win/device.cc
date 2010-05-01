// win/device.cc

#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>			// why is this needed first? problems with including our stuff first!
#include <audio/device.hp>
#include <audio/buffer.hp>
#include <pigsys/pigsys.hp>

#pragma comment( lib, "dsound.lib" )

SoundDevice::SoundDevice( HWND hwnd )
{
// kts removed 4/20/2002 7:10AM, asserts on DS_OK under windows2000
// will address when we actually implement sound
//	if ( !hwnd )
//	{
//		extern HWND worldFoundryhWnd;
//		hwnd = worldFoundryhWnd;
//	}

//	assert( hwnd );

//	HRESULT hr;

//	hr = DirectSoundCreate( NULL, &lpDirectSound, NULL );
//	assert( hr == DS_OK );
//   	lpDirectSound->SetCooperativeLevel( hwnd, DSSCL_PRIORITY );
}


SoundDevice::~SoundDevice()
{
//	assert( lpDirectSound );
//   	HRESULT hr = lpDirectSound->Release();
//	assert( hr == DS_OK );
}


SoundBuffer*
SoundDevice::CreateSoundBuffer( binistream& binis )
{
//	return new SoundBuffer( lpDirectSound, binis );
	return NULL;
}

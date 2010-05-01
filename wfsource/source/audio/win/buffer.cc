// buffer.cc

#define STRICT
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>			// why is this needed first? problems with including our stuff first!
#include <audio/buffer.hp>
#include <pigsys/pigsys.hp>

SoundBuffer::SoundBuffer( LPDIRECTSOUND& lpDirectSound, binistream& binis ) :
	_binis( binis )
{
	assert( lpDirectSound );

	HRESULT hr;

	const WAV_HEADER = 0x5A;

    PCMWAVEFORMAT pcmwf;
    // Set up wave format structure.
    memset( &pcmwf, 0, sizeof( pcmwf ) );
    pcmwf.wf.wFormatTag = WAVE_FORMAT_PCM;
    pcmwf.wf.nChannels = 2;
    pcmwf.wf.nSamplesPerSec = 22050;
//    pcmwf.wf.nSamplesPerSec = 11025;
    pcmwf.wf.nBlockAlign = 4;
    pcmwf.wf.nAvgBytesPerSec = pcmwf.wf.nSamplesPerSec * pcmwf.wf.nBlockAlign;
    pcmwf.wBitsPerSample = 16;

    DSBUFFERDESC dsbdesc;
    // Set up DSBUFFERDESC structure.
    memset( &dsbdesc, 0, sizeof( dsbdesc ) ); // Zero it out.
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
    dsbdesc.dwBufferBytes = _binis.getFilelen() - WAV_HEADER;
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;

   	hr = lpDirectSound->CreateSoundBuffer( &dsbdesc, &lplpDsb, NULL );
	assert( hr == DS_OK );

	LPVOID lpvPtr1, lpvPtr2;
	DWORD dwBytes1, dwBytes2;
	// Obtain write pointer
	hr = lplpDsb->Lock( 0, dsbdesc.dwBufferBytes, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0 );
	assert( hr == DS_OK );
	assert( ValidPtr( lpvPtr1 ) );

	// Write to pointers
	byte* wave = (byte*)_binis.GetMemoryPtr( dsbdesc.dwBufferBytes ) + WAV_HEADER;
	assert( ValidPtr( wave ) );
    CopyMemory( lpvPtr1, wave, dwBytes1 );
    if ( lpvPtr2 )
        CopyMemory( lpvPtr2, wave+dwBytes1, dwBytes2 );

    hr = lplpDsb->Unlock( lpvPtr1, dwBytes1, lpvPtr2, dwBytes2 );
	assert( hr == DS_OK );
}


SoundBuffer::~SoundBuffer()
{
	assert( lplpDsb );

	HRESULT hr;

	hr = lplpDsb->Stop();
	assert( hr == DS_OK );

	hr = lplpDsb->Release();
	assert( hr == DS_OK );
}


void
SoundBuffer::play() const
{
	assert( lplpDsb );

	HRESULT hr = lplpDsb->Play( 0, 0, 0 );
	assert( hr == DS_OK );
}

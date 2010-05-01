//==============================================================================
// sampprof.cc: psx profiler:
//==============================================================================

#include <pigsys/pigsys.hp>
#include <cstdlib>
#include <memory.h>
#include <pigsys/genfh.hp>
#include <hal/hal.h>

#if defined( __PSX__ )

extern "C" {
//#include <sys\types.h>
//#include <cstddef>
//#include <r3000.h>
//#include <libgte.h>
//#include <libgpu.h>
//#include <cstdio>
//#include <asm.h>
#include <kernel.h>
//#include <libsn.h>
//#include <libetc.h>
//#undef DO_ASSERTIONS
//#define DO_ASSERTIONS 1
//#include <missing.h>
}

#include <profile/sampprof.hp>

#define SysToT	((struct ToT*)0x100)

extern "C" unsigned long __text;
extern "C" unsigned long __textlen;

//#if !defined(PROFILE)
//#error what the hell are you using this for if not for profile?
//#endif

//==============================================================================

volatile static profileSample* theSampler;

static long int
samplerInterruptHandler(void)
{
	struct TCBH* h = (struct TCBH*)SysToT[ 1 ].head;
	struct TCB* p = h->entry;

	assert( theSampler );
//	printf("int: thesampler = %p\n",theSampler);
	theSampler->hit( (void*)p->reg[ R_EPC ] );
	return( 0 );
}

//==============================================================================

profileSample::profileSample( int nSampleRate )
	{
	assert( theSampler  == NULL);
	theSampler = this;
	assert( theSampler );

	printf("thesampler = %p\n",theSampler);

	_nSampleRate = nSampleRate;

	romHits = 0;
	programHits = new (HALLmalloc) unsigned long[ __textlen];
	if(programHits == 0 || (signed long)programHits == -1)
	 {
		printf("profileSample:profileSample: cannot allocate profile memory, aborting\n");
		printf(" needed %ul bytes\n",__textlen);
		sys_exit(5);
	 }
	assert( ValidPtr(programHits) );
//	AssertMsg( AlignedPtr(programHits+(__textlen/sizeof(long))), "end ptr = " << programHits+(__textlen/sizeof(long)));
	assert( AlignedPtr(programHits+(__textlen/sizeof(long))));
	assert( InMappedMemory(programHits+(__textlen/sizeof(long))));
//	AssertMsg( ValidPtr(programHits+(__textlen/sizeof(long))), "programHits = " << programHits << ", __textlen = " << __textlen );
	assert( ValidPtr(programHits+(__textlen/sizeof(long))));
	assert( ValidPtr(programHits+(__textlen/sizeof(long))));
	assert( (signed long)programHits != -1 );
	memset( (unsigned char*)programHits, 0, __textlen );

	long int (*ihPtr)() = samplerInterruptHandler;
	unsigned long foo = (unsigned long)ihPtr;
	foo |= 0xa0000000;
	ihPtr = (long int (*)())foo;
	EnterCriticalSection();
//	unsigned long EHbl = OpenEvent( RCntCNT1, EvSpINT, EvMdINTR, samplerInterruptHandler | 0xa0000000);
	unsigned long EHbl = OpenEvent( RCntCNT1, EvSpINT, EvMdINTR, ihPtr);
	EnableEvent( EHbl );
	SetRCnt( RCntCNT1, nSampleRate, RCntMdINTR );
	ResetRCnt( RCntCNT1 );
	StopRCnt( RCntCNT1 );
	ExitCriticalSection();
	}

//==============================================================================

profileSample::~profileSample()
	{
	stop();
	assert( programHits );
	HALLmalloc.Free(programHits);
	theSampler = NULL;
	}

//==============================================================================

void
profileSample::hit( void* addr )
	{
	unsigned long l = (unsigned long)addr;
	assert( (l % 4) == 0 );
	assert( (__text % 4) == 0 );

	if ( __text <= l && l < (__text + __textlen) )
	{
		unsigned long* hit = &programHits[ (l - __text) / sizeof(long) ];
		assert(hit >= programHits);
		assert(hit < programHits + __textlen/sizeof(long));
//		AssertMsg(ValidPtr(hit), "hit = " << hit );
		assert(ValidPtr(hit));
		++(*hit);
	}
	else
		++romHits;
	}

//==============================================================================

void
profileSample::start()
	{
	assert( theSampler );
	printf("profileSample:profileSample: starting\n");
	ResetRCnt( RCntCNT1 );
	StartRCnt( RCntCNT1 );
	}

//==============================================================================

void
profileSample::stop()
	{
	StopRCnt( RCntCNT1 );
	}

//==============================================================================

void
profileSample::save( const char* szProfileFilename )
	{
	printf("profileSample::save: saving\n");

	int fh = FHOPENWR( szProfileFilename );
	assert( fh != -1 );

	unsigned long tag = 'SAMP';
	FHWRITE( fh, &tag, 4 );
	unsigned long size = 3*4 + __textlen;
	FHWRITE( fh, &size, 4 );
	FHWRITE( fh, &romHits, 4 );
	FHWRITE( fh, &_nSampleRate, 4 );
	FHWRITE( fh, &__text, 4 );
	FHWRITE( fh, &__textlen, 4 );

	ASSERTIONS( unsigned int cbWritten = ) FHWRITE( fh, programHits, __textlen );
	assert( cbWritten == __textlen );

	FHCLOSE( fh );
	printf("profileSample::save: done saving\n");
	}

//==============================================================================
#endif
//==============================================================================

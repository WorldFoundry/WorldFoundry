//=============================================================================
// time.cc: clock reading funcions
//=============================================================================
// Time Clock API
//
//			SYS_TICKS returns a number of ticks, as a long,
// 					and is used by the Streamer
//
//			SYS_CLOCK returns a number of seconds since reset, as a Scalar
//
//			SYS_CLOCK_RESET resets the clock to zero
//
//=============================================================================

#include <hal/time.hp>

long _nWallClockBaseTime;

//=============================================================================

#ifdef __PSX__
Scalar SYS_CLOCK(void)
{
	Scalar result;
	ulong time = _PigsRootCounter2 - _nWallClockBaseTime;
	result = Scalar((int32)time << 9);  // *65536/128

	// 11/25/96 9:08PM brm hack: Timed PSX. 1000 PSX seconds = 551 real seconds.
	// I'm not sure why this is occuring, but I was unable to find the cause,
	// so here we are:

	result = result * SCALAR_CONSTANT( 551.0 / 1000.0 );

//	printf("better2 _PigsRootCounter2 = %lx, wallclockbasetime = %lx,result = %lx\n", _PigsRootCounter2,_nWallClockBaseTime,br_scalar(result));
//	printf("time = %08lx\n",time);
	return(result);
}
#endif

//=============================================================================

#ifdef __DOS__
#include <bios.h>
#define SYS_TICKS()		getMsdosTimeOfDay()

long getMsdosTimeOfDay( void )
{
	long	msdosTimeOfDay;
	_bios_timeofday( _TIME_GETCLOCK, &msdosTimeOfDay );
	return( msdosTimeOfDay );
}

Scalar SYS_CLOCK(void)
{
	Scalar Xtime =  Scalar( ( (float)(getMsdosTimeOfDay()-_nWallClockBaseTime) )*(65536/18.2) );
//	assert( Xtime >= Scalar::zero );
	return Xtime;
}
#endif

//=============================================================================

#if defined( __WIN__ )

#endif

//=============================================================================

void
SYS_CLOCK_RESET(void)
{
	_nWallClockBaseTime = SYS_TICKS();
}

//#ifdef __DOS__
//extern uint32	_pigsDosTimerValue;
//#define SYS_TICKS()		(_pigsDosTimerValue)
//#endif

//=============================================================================

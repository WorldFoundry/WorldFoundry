//=============================================================================
// platform.h: platform specific includes
//=============================================================================
// use only once insurance

#ifndef __PLATFORM_H
#define __PLATFORM_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Machine specific includes
//	History:
//			Created	07-08-96 07:21pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#include "_procsta.h"


//==============================================================================
// kts clean up all of HAL and re-organize it
// this wants to be in a platform specific include which is visible to all

// macros for PlayStation scratch pad

//#define getScratchAddr(offset)  ((u_long *)(0x1f800000+offset*4))
// use getScratchAddr to calc a scratchpad address in longs
// defined in libetc.h

#define SetSPadStack(addr) {						\
    	__asm__ volatile("move $8,%0"	::"r"(addr):"$8","memory");	\
	__asm__ volatile("sw $29,0($8)"	::	   :"$8","memory");	\
	__asm__ volatile("addiu $8,$8,-4" ::	   :"$8","memory");	\
	__asm__ volatile("move $29,$8"	::	   :"$8","memory");	\
}

#define ResetSPadStack() {						\
	__asm__ volatile("addiu $29,$29,4"  ::     :"$29","memory");	\
	__asm__ volatile("lw $29,0($29)"    ::	   :"$29","memory");	\
}

#define GetStackAddr(addr) {						\
	__asm__ volatile("move $8,%0"	::"r"(addr):"$8","memory");	\
	__asm__ volatile("sw $29,0($8)"	::	   :"$8","memory");	\
}


extern bool _scratchPadInUse;

//getScratchAddr(offset)

INLINE void
LockScratchPad()
{
	assert(!_scratchPadInUse);
	_scratchPadInUse = true;
}


INLINE void
UnlockScratchPad()
{
	assert(_scratchPadInUse);
	_scratchPadInUse = false;
}

extern LMalloc* scratchPadMemory;

//=============================================================================
#endif
//=============================================================================


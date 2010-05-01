//============================================================================
// _procsta.h: multi-tasker interface, procState
//============================================================================
// use only once insurance

#ifndef __PROCSTATE_H
#define __PROCSTATE_H

//============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Machine specific portion of tasker
//	History:
//			Created	12-06-94 06:07pm Kevin T. Seghetti
//			(in progress)

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>

//=============================================================================
// SProcState declaration, this is the processor specific portion
// might put these into seperate includes or something

#ifndef	PROC_STACKSIZE
#define	PROC_STACKSIZE	(1024*16)
#endif	//!defined(PROC_STACKSIZE)

typedef struct _procState
{
	void* _stackBase;
	int32 _stackSize;
	// this is entirely processor specific, contains registers and flags
    // this is entirely platform specific, contains a Task Control Block handle
// PSX specific
    uint32 _TCBhandle;
} SProcState;

//-----------------------------------------------------------------------------
// debugging macros

#if DO_ASSERTIONS
#define VALIDATEPROCSTATE(proc) \
	VALIDATEPTR(proc)
#else
#define VALIDATEPROCSTATE(proc)
#endif

//-----------------------------------------------------------------------------

void
ProcStateConstruct(SProcState* self, voidFunction* routine);

void
ProcStateDestruct(SProcState* self);

//=============================================================================
#endif
//=============================================================================

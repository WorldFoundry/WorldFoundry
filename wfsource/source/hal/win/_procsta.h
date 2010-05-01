//=============================================================================
// _procsta.h: multi-tasker interface, procState
//=============================================================================
// use only once insurance

#ifndef __PROCSTATE_H
#define __PROCSTATE_H

//=============================================================================
// Documentation:

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
#define	PROC_STACKSIZE	(0x10000)
#pragma message ("_procsta.h: kts adjust stack size")
#endif	//!defined(PROC_STACKSIZE)


typedef struct _procState
{
	void* _stackBase;
	int32 _stackSize;
	// this is entirely processor specific, contains registers and flags
// 386 specific
	voidFunction* _eip;
	void* _esp;

	int32 _efl;
	int32 _eax;
	int32 _ebx;
	int32 _ecx;
	int32 _edx;
	int32 _esi;
	int32 _edi;
	int32 _ebp;
//    int32 _cs;     CS, DS, ES, and SS are assumed by
//    int32 _ds;     the compiler to not change, so do
//    int32 _es;     not save and restore them during
//    int32 _ss;     task switches
    int32 _fs;
    int32 _gs;
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

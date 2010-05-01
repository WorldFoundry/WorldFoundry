//=============================================================================
// _ProcSta.cc: multi-tasker code, processor state, win/80386 specific code
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:

//	History:
//			Created	12-06-94 05:52pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:
//	Example:
//=============================================================================
// dependencies

#define __pROCSTATE_C
#include "_procsta.h"
#include <hal/tasker.h>
#include <hal/_tasker.h>
#include <hal/salloc.hp>

//=============================================================================

extern SAlloc* stacks;

//=============================================================================

//
//extern unsigned short GetFS( void );
//#pragma aux GetFS = \
//       "mov ax,fs"   \
//       value   [ax]  \
//       modify  [ax];
//
//extern unsigned short GetGS( void );
//#pragma aux GetGS = \
//       "mov ax,gs"   \
//       value   [ax]  \
//       modify  [ax];


#pragma message (__FILE__ " KTS: dummy, needs to be written")
unsigned short GetFS(void)
{
		return(0);
}

void
ProcStateConstruct(SProcState* self,voidFunction* routine)
{
	unsigned long ugp;

	VALIDATEPTR(self);
	VALIDATEPTR(routine);

	self->_stackSize = PROC_STACKSIZE;
//	self->_stackBase = malloc(self->_stackSize);
	self->_stackBase = stacks->Allocate(self->_stackSize);
	assert( self->_stackBase );
	self->_eip = routine;
	self->_esp = ((int8*)self->_stackBase) + (self->_stackSize-4);

	self->_efl = 0x3212;
	self->_eax = 0;
	self->_ebx = 0;
	self->_ecx = 0;
	self->_edx = 0;
	self->_esi = 0;
	self->_edi = 0;
	self->_ebp = 0;
    self->_fs  = GetFS();
    self->_gs  = 0;
}

//=============================================================================

void
ProcStateDestruct(SProcState* self)
{
	VALIDATEPROCSTATE(self);
//	free(self->_stackBase);
	stacks->Free(self->_stackBase);
}

//=============================================================================

//=============================================================================
// _ProcSta.c: multi-tasker code, processor state, 80386 specific code
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

extern "C" {
#include <r3000.h>
#include <asm.h>
#include <kernel.h>
#include <cstdio>
#include <missing.h>
};

//=============================================================================

#if DO_TEST_CODE

char* tcbStatusStrings[4] =
{
	"Unused ",
	"Unknown",
	"Unknown",
	"Active "
};

void
DumpThreads(void)
{
	struct ToT *tot = (struct ToT*)0x00000100;
	struct TCB* tcb,*tcb_tab;
	struct TCBH *tcbh_tab;

	int i,status;

	ERR_DEBUG(("Thread Dump:\n"));
	// display the threads related part of ToT

	tcb_tab = (struct TCB*) (tot+2)->head;
	tcbh_tab = (struct TCBH*) (tot+1)->head;

	for(i=0;i<HAL_MAX_TASKS;i++)
	 {
		tcb = &tcb_tab[i];

		if(tcbh_tab->entry == tcb)
			ERR_DEBUG(("-->"));
		else
			ERR_DEBUG(("   "));

		ERR_DEBUG(("TCB %d address = $%8X, ",i,tcb));

		if(tcb->status!=TcbStUNUSED && tcb->status!=TcbStACTIVE)
			status = 2;
		else
			status = (tcb->status>>12)-1;
		assert(status < 4);
		ERR_DEBUG(("Status = $%4X (%s), ",tcb->status,tcbStatusStrings[status]));
		ERR_DEBUG(("Mode = $%8X\n",tcb->mode));
	 }
	ERR_DEBUG(("TCBH entry = $%8X\n\n",tcbh_tab->entry));
}
#endif

//=============================================================================

#pragma warning( "KTS: This no longer compiles -- should it be removed here or from 'missing.h' or ???" )
//typedef long (*longFunction)();

void
ProcStateConstruct(SProcState* self,voidFunction* routine)
{
	unsigned long ugp;

	VALIDATEPTR(self);
	VALIDATEPTR(routine);

	__asm__ ("move %0,$gp": "=r" (ugp):);

	self->_stackSize = PROC_STACKSIZE;
	self->_stackBase = new (HALLmalloc) char[self->_stackSize];
	assert(ValidPtr(self->_stackBase));
	AssertMemoryAllocation(self->_stackBase);

    EnterCriticalSection();
    self->_TCBhandle = OpenTh( (longFunction)routine, (unsigned long)self->_stackBase+self->_stackSize, ugp);
    assert(self->_TCBhandle != -1);
    ExitCriticalSection();
#if DO_TEST_CODE
//	DumpThreads();
#endif
}

//=============================================================================

void
ProcStateDestruct(SProcState* self)
{
	VALIDATEPROCSTATE(self);

    EnterCriticalSection();
    CloseTh( self->_TCBhandle);
    ExitCriticalSection();

	free(self->_stackBase);
}

//=============================================================================

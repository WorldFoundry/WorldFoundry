//=============================================================================
// taskibm.c: platform specific multi-tasker code
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:

//	History:
//			Created	02-28-95 12:09pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:
//	Example:

//=============================================================================
// dependencies

#define __tASKIBM_C
#include "_procsta.h"
#include <hal/tasker.h>
#include <hal/_tasker.h>
#include <cpplib/stdstrm.hp>
#include <cstdio>

//=============================================================================
// TaskSwitch jumps to here

extern "C" void
_Reschedule(void)					// execution comes here from TaskSwitch
{
	_ActivateNextTask();			// make highest priority task the current task
	_TaskerDispatchCurrentTask();	// pass control to current task, never returns
	assert(0);
}

//=============================================================================
// sets up tasking system, executed by PIGS startup code
// WARNING: _TaskerStart initializes the multi-tasking system, it must only
// be called once upon startup

void
_TaskerStart(voidFunction* firstTask,int maxTasks, int /*maxMessages*/ , int /*maxPorts*/)
{
	VALIDATEPTR(firstTask);
	ListConstruct(&tasksReady);
	ListConstruct(&tasksWaiting);

	memPoolTasks = MemPoolConstruct(sizeof(STask),maxTasks,HALLmalloc);

	// kts must have at least one running task for now
	currentTask = (STask*)MemPoolAllocate(memPoolTasks,sizeof(STask));
	_TaskConstruct(currentTask,firstTask,NULL);

	currentTask->_state = ETASKSTATE_READY;
	ListEnqueue(&tasksReady, (SNode*)currentTask,NodeGetPriority(&currentTask->_link));

	_TaskerSaveSysRegsAndDispatchCurrentTask();		// begin multi-tasking
	// will return to here when TaskerEnd is executed
}

//=============================================================================
// WARNING: _TaskerEnd shuts down the multi-tasking system, it must only
// be called once upon exit

void
_TaskerStop(void)
{
	ITask iCurrent = GetCurrentTask();

	_TaskDelete(iCurrent);

//	assert(ListEmpty(&tasksWaiting));
//	assert(ListEmpty(&tasksReady));

	DBSTREAM1( if(!ListEmpty(&tasksWaiting))
		cwarn << "WARNING: TaskerStop called with tasks still waiting\n"; )
	DBSTREAM1( if(!ListEmpty(&tasksReady))
		cwarn << "WARNING: TaskerStop called with tasks still running\n"; )

	MemPoolDestruct(memPoolTasks);

	ListDestruct(&tasksWaiting);
	ListDestruct(&tasksReady);
	_TaskerRestoreSysRegs();
}

//=============================================================================

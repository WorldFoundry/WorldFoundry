//=============================================================================
// taskpsx.c: psx specific multi-tasker code
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

#define __tASKPSX_C
#include <_procsta.h>
#include <hal/tasker.h>
#include <hal/_tasker.h>
#include <hal/hal.h>

extern "C" {
#	include <kernel.h>
	};

#include <missing.h>
#include <cstdio>

static long	thread_id_0 = 0xFF000000;

//=============================================================================
// TaskSwitch jumps to here

void
TaskSwitch(void)				// execution comes here from TaskSwitch
{
	_ActivateNextTask();		// make highest priority task the current task
//	DumpThreads();
//	ERR_DEBUG(("Switching to task $%8X\n",currentTask->_taskState._TCBhandle));

	ChangeTh(currentTask->_taskState._TCBhandle);		// pass control to current task, never returns
	// when this task becomes re-activated, will return to here, which will return to caller
}

//=============================================================================
// sets up tasking system, executed by PIGS startup code
// WARNING: _TaskerBegin initializes the multi-tasking system, it must only
// be called once upon startup

void
_TaskerStart(voidFunction* firstTask,int maxTasks, int maxMessages, int maxPorts)
{
	(void)maxMessages;
	(void)maxPorts;
	VALIDATEPTR(firstTask);
	ListConstruct(&tasksReady);
	ListConstruct(&tasksWaiting);

	memPoolTasks = MemPoolConstruct(sizeof(STask),maxTasks, HALLmalloc);

	assert(maxTasks == HAL_MAX_TASKS);	// kts HAL_MAX_TASKS is used in platform.c to set up the psx config, so must be same

	// kts must have at least one running task for now
	currentTask = (STask*)MemPoolAllocate(memPoolTasks,sizeof(STask));
	_TaskConstruct(currentTask,firstTask,NULL);

	currentTask->_state = ETASKSTATE_READY;
	ListEnqueue(&tasksReady, (SNode*)currentTask,NodeGetPriority(&currentTask->_link));

	// kts note: need some way to save current TCB, I just don't know how yet
	ChangeTh(currentTask->_taskState._TCBhandle);		// pass control to current task, never returns
	ERR_DEBUG_LOG(ERR_HAL_PROGRESS, ("HAL: Tasker shutting down\n"));

	// should
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

	assert(ListEmpty(&tasksWaiting));
	assert(ListEmpty(&tasksReady));

	MemPoolDestruct(memPoolTasks);

	ListDestruct(&tasksWaiting);
	ListDestruct(&tasksReady);
	ChangeTh(thread_id_0);
	assert(0);				// should never get here
}

//=============================================================================

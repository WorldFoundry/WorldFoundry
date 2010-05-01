//=============================================================================
// _tasker.h: private tasker interface
//=============================================================================
// use only once insurance

#ifndef __tASKER_H
#define __tASKER_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//		Since the tasker is split into several files, need a private .h file
//		This is the private portion of the tasker interface
//	History:
//			Created	01-03-95 09:34am Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//		ProcState, List

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

//#include <hal/_procsta.h>
#include <hal/platform.h>			        // get from correct platform specific directory
#include <hal/_list.h>
#include <hal/_mempool.h>
#include <hal/tasker.h>

//=============================================================================
// WARNING: _TaskerStart initializes the multi-tasking system, it must only
// be called once upon startup

void
_TaskerStart(voidFunction* firstTask, int maxTasks,int maxMessages, int maxPorts);

//=============================================================================
// WARNING: _TaskerStop shuts down the multi-tasking system, it must only
// be called once upon exit

void
_TaskerStop(void);

//=============================================================================
// private functions prototypes

// executed from TaskSwitch

void
_TaskInit(int maxTasks, int maxMessages, int maxPorts);

void
_ActivateNextTask(void);

ITask
_TaskConstruct(STask* self,voidFunction* routine,void* userData);

void
_TaskDestruct(STask* self);

ITask
_TaskDelete(ITask iSelf);

//=============================================================================
// STask declaration

#define TASKNAMELEN 16

struct _Task
{
	SNode _link;
	SProcState _taskState;		// processor state storage
	// kts note, don't change this struct up to here unless you sync it with task386.asm
	ITask _iSelf;
	char _name[TASKNAMELEN];
	int8 _flags;
	int8 _state;				// ETASKSTATE_READY or ETASKSTATE_WAITING running or waiting

	int32 _sigWait;
	int32 _sigAlloc;
	int32 _sigRecieved;

	short _forbidCount;			// if not zero, task switching forbidden
	short _disableCount;			// if not zero, interrupts forbidden

	void* _userData;			// points to user data, if any
};

//=============================================================================
// enums

typedef enum 		// task state
{
	ETASKSTATE_INVALID = 0,			// invalid state
	ETASKSTATE_READY,				// send signal to task when message arrives
	ETASKSTATE_WAITING,
	ETASKSTATE_KILLED
} TaskState;

extern SList tasksReady;					// prioritized list of tasks wanting cpu
extern SList tasksWaiting;					// list of tasks waiting for messages

extern SMemPool* memPoolTasks;

//=============================================================================
// functions

extern "C" void
_TaskerSaveSysRegsAndDispatchCurrentTask(void);


extern "C" void
_TaskerRestoreSysRegs(void);


// this is in assembly, and actually tranfers control to the CurrentTask
// therefore, this function does not return

extern "C" void
_TaskerDispatchCurrentTask(void);

//=============================================================================
// data

extern "C" STask* currentTask;			 				// currently executing task

//=============================================================================

#if TEST_TASKER
void
TaskerTest(void);
#endif

//=============================================================================
#endif
//=============================================================================

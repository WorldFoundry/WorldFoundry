//=============================================================================
// tasker.h: PIGS multi-tasker interface
//=============================================================================
// use only once insurance

#ifndef _tASKER_H
#define _tASKER_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			The PIGS system is designed around a cooperative/pre-emptive
//			multi-tasking system.  This is the interface to the tasker.
//	History:
//			Created	10-24-94 10:19am Kevin T. Seghetti
//			(in progress)(Pre-emption is not yet implemented)

//	Class Hierarchy:
//			none

//	Dependancies:
//			SList,SNode,SMessage,ProcState

//	Restrictions:
//			The tasker needs to be started before it can be used, see game.c
//			for an example of how to do this

//			For now, tasks should not return from their entry point, they
//			will blow up

//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>
#include <hal/item.h>

//=============================================================================

struct _Task;
typedef struct _Task STask;

ITEMTYPECREATE(ITask,STask);

//=============================================================================
// debugging macros
// validates task item

#if DO_ASSERTIONS
#define VALIDATETASKPTR(task) \
	{\
	VALIDATEPTR(task); \
	VALIDATEPROCSTATE((&(task)->_taskState)); \
	if((task) != currentTask)\
		VALIDATENODE((&(task)->_link));\
	}

// validates task pointer

#define VALIDATETASK(iTask) \
 { \
	STask* task; \
	VALIDATEITEM(iTask); \
	task = ITEMRETRIEVE(iTask,STask); \
	VALIDATETASKPTR(task); \
 }
#else
#define VALIDATETASKPTR(task)
#define VALIDATETASK(iTask)
#endif

//=============================================================================
// global functions


ITask
GetCurrentTask(void);		// returns ITask for currently executing task

// spawns a new task, will transfer control if new task has a higher priority
ITask
CreateTask(voidFunction* start,short priority, void* taskData);

void
TaskKill(ITask iSelf);		// Immediatly shut task down, this will not free
							// any resources Task allocated(although items in
							// debugging mode will notifly you if all resources
						    // for this task were not freed

extern "C"
void TaskSwitch(void);		// cause a task-switch to occur, if another task
							// of equal or higher priority exists, will cause
							// that task to gain the CPU

//=============================================================================
// functions which operate on any task

void*
TaskGetUserData(ITask iSelf);

short
TaskGetPriority(ITask iSelf);				// read task priority of this

void
TaskSetPriority(ITask iSelf,short priority); // set task priority of this

const char*
TaskGetName(ITask iSelf);                    // get const ptr to task name

void
TaskSetName(ITask iSelf,char* name);         // set task name

//=============================================================================
// note: use these with caution, prolonged Disables can adversly affect
//	the system

void
DisableTaskSwitching(void);				// lock out task switching

void
EnableTaskSwitching(void);				// re-enable task switching

void
DisableInterrupts(void);				// turn interrupts off

void
EnableInterrupts(void);					// turn interrupts back on

//=============================================================================
#endif
//=============================================================================

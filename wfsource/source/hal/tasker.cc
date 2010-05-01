//============================================================================
// Tasker.c: multi-tasker code, 80386 version
//============================================================================
// Documentation:
//=============================================================================
//	Abstract:

//	Flow:

// 		A Task Switch:

//		User Task A is running, executes TaskSwitch
//		TaskSwitch runs (in assembly), saving current context, then
//		jumps to _Reschedule, a C function which chooses highest priority task to run
//		Which then calls _DispatchTask, which actually restores the context of the new task
//		Now User Task B is running






//	History:
//			Created	12-02-94 11:41am Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//			halbase.h
//			_list.h

//	Restrictions:
//		tasker curently doesn't handle having no running tasks
//	Example:


// tasker functions should assert that tasker is running
//=============================================================================
// dependencies


#define _tASKER_C
#include <hal/hal.h>
#include <hal/_tasker.h>
#include <hal/_signal.h>
#include <hal/_message.h>
#include <hal/_mempool.h>

//#include <cstdio>

//=============================================================================
// regional

extern "C" STask systemState;
STask  systemState;	 				// context in which tasker code can execute
extern "C" STask* currentTask;
STask* currentTask;	 				// currently executing task(also in taskReady list
SList tasksReady;					// prioritized list of tasks wanting cpu
SList tasksWaiting;					// list of tasks waiting for messages

SMemPool* memPoolTasks;

#if defined( _MSC_VER )
void
TaskSwitch()
{
}

void
_TaskerDispatchCurrentTask()
{
}

void
_TaskerSaveSysRegsAndDispatchCurrentTask()
{
}

void
_TaskerRestoreSysRegs()
{
}
#endif


//=============================================================================
// private functions

//=============================================================================
// select highest priority task as current task, only called by _Reschedule
// this may lead to the same task being current, if it is the highest priority
// and there are no other equal pririty tasks waiting to run

void
_ActivateNextTask(void)
{
	STask* readyTask;

	if(ListEmpty(&tasksReady))
	 {
		ERR_DEBUG(("No Ready Tasks, exiting\n"));
		printf("No Ready Tasks, exiting\n");
                sys_exit(0);
	 }

//	 this is to cause round-robin execution if more than one task of same pri
//	 only should happen if current task is at top of ready task list
	if(currentTask == (STask*)ListGetHead(&tasksReady))
	 {
		NodeRemove((SNode*)currentTask);
		ListEnqueue(&tasksReady, (SNode*)currentTask,NodeGetPriority(&currentTask->_link));
	 }

	// select highest priority task as current task
	readyTask = (STask*)ListGetHead(&tasksReady);

	if(readyTask
		&&
		 (	NodeGetPriority(&readyTask->_link) >= NodeGetPriority(&currentTask->_link)
			|| currentTask->_state != ETASKSTATE_READY
		 )
	  )
	 {
		// do task switch
		currentTask = readyTask;
	 }
}

//=============================================================================
// only used by TaskStart and CreateTask

ITask
_TaskConstruct(STask* self,voidFunction* routine, void* userData)
{
	VALIDATEPTR(self);
	VALIDATEPTR(routine);
	NodeConstruct(&self->_link);

	self->_flags = 0;
	self->_state = ETASKSTATE_INVALID;				// invalid state
	self->_sigWait = 0;
	self->_sigAlloc = 0;
	self->_sigRecieved = 0;

	self->_forbidCount = 0;
	self->_disableCount = 0;

	self->_iSelf = ITEMCREATE(self,STask);

	self->_name[0] = 0;
	self->_userData = userData;

	ProcStateConstruct(&self->_taskState,routine);
	return(self->_iSelf);
}

//=============================================================================

void
_TaskDestruct(STask* self)
{
	VALIDATETASKPTR(self);
	NodeRemove(&self->_link);
	NodeDestruct(&self->_link);

	ProcStateDestruct(&self->_taskState);
}

//=============================================================================

ITask
_TaskDelete(ITask iSelf)
{
	STask* self;
	VALIDATEITEM(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	ITEMDESTROY(iSelf,STask);
	VALIDATETASKPTR(self);
	_TaskDestruct(self);
	MemPoolFree(memPoolTasks,self);
	return(NULLITEM);
}

//=============================================================================
// public functions
//=============================================================================

void*
TaskGetUserData(ITask iSelf)
{
	STask* self;
	VALIDATETASK(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	assert(self->_userData);			// if user data ptr is NULL, user should'nt try to get ptr
	return(self->_userData);
}

//=============================================================================

short
TaskGetPriority(ITask iSelf)
{
	STask* self;
	VALIDATETASK(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	return(NodeGetPriority(&self->_link));
}

//=============================================================================

void
TaskSetPriority(ITask iSelf,short priority)
{
	STask* self;
	VALIDATETASK(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	NodeSetPriority(&self->_link,priority);
}

//=============================================================================

const char*
TaskGetName(ITask iSelf)
{
	STask* self;
	VALIDATETASK(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	return((const char*)self->_name);			// kts removed & 07-08-96 04:44pm
//	return((const char*)&self->_name);
}

//=============================================================================

void
TaskSetName(ITask iSelf,char* name)
{
	STask* self;
	VALIDATETASK(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	VALIDATESTRING(name);
	assert(strlen(name) < TASKNAMELEN);
	StringCopyCount(self->_name,name,TASKNAMELEN);
}

//=============================================================================

void
TaskKill(ITask iSelf)				// shut task down
{
	STask* self;
	VALIDATETASK(iSelf);
	self = ITEMRETRIEVE(iSelf,STask);
	self->_state = ETASKSTATE_KILLED;
#if defined( DO_MULTITASKING )
	_TaskDelete(iSelf);					// KTS BUG: freeing stack while still in use if same task
		// N.B.: MemCheck chokes on this
#endif
	if(currentTask == self)
	 {
		TaskSwitch();				// should not return
		assert(0);
	 }
}

//=============================================================================
// globals
//-----------------------------------------------------------------------------

ITask
GetCurrentTask(void)		// returns task item for self
{
	assert( currentTask != NULL );
	return((ITask)currentTask->_iSelf);
//	return(ITEMLOOKUP(currentTask,ITask));
}

//=============================================================================
// all tasks run as a sub-routine to this routine, allowing them to return

static voidFunction* _taskRoutine;
static short _priority;

static void
TaskShell(void)
{
	TaskSetPriority(GetCurrentTask(),_priority);
	(*_taskRoutine)();
	DBSTREAM2(cwarn << "WARNING: Task returned to caller\n"));
	TaskKill(GetCurrentTask());
}

//=============================================================================
// spawns a new task, will transfer control if new task has a higher or equal priority

ITask
CreateTask(voidFunction* startRoutine, short priority, void* userData)
{
	STask* newTask;
	ITask iNewTask;

	VALIDATEPTR(startRoutine);

	newTask = (STask*)MemPoolAllocate(memPoolTasks,sizeof(STask));
	iNewTask = _TaskConstruct(newTask,&TaskShell,userData);

	newTask->_state = ETASKSTATE_READY;
	ListEnqueue(&tasksReady, (SNode*)newTask,PRIORITY_SYSTEM_CRITICAL);
	_priority = priority;
	_taskRoutine = startRoutine;
	TaskSwitch();

//	if(priority > NodeGetPriority(&currentTask->_link))
//		TaskSwitch();
	return(iNewTask);
}

//=============================================================================
// note: use these with caution, prolonged Disables can adversly affect
//	the system

void
DisableTaskSwitching(void)			// lock out task switching
{
	currentTask->_forbidCount++;
}

//=============================================================================

void
EnableTaskSwitching(void)			// re-enable task switching
{
	if(currentTask->_forbidCount > 0)
		currentTask->_forbidCount--;
	if(!currentTask->_forbidCount)
		TaskSwitch();					// in case current task is not highest priority
}

//=============================================================================

#if 0
void
DisableInterrupts(void)			// turn interrupts off
{
	UNIMPLEMENTED();
}

//=============================================================================

void
EnableInterrupts(void)			// turn interrupts back on
{
	UNIMPLEMENTED();
	TaskSwitch();
}
#endif

//=============================================================================
// test suite
// kts note: needs to test task priority

#if TEST_TASKER

#include <cstdio>
#include <cstdlib>

void
TestTask2(void)
{
	int counter = 0;
	// this is a very boring task, it just increments a local counter
	DBSTREAM2(cprogress << "Task 2 Started\n");

	while(1)
	 {
		counter++;
		DBSTREAM2(cprogress << "Test Task2 Running\n");
		if(counter == 50)
		 {
			DBSTREAM2(cprogress << "task 2 killing self\n");
			break;						// here we test when a task exits its initial function
		 }
		TaskSwitch();
	 }
}

//=============================================================================

void
TestTask3(void)
{
	DBSTREAM2(cprogress << "Task 3 Started\n");
	// this is a very boring task, it just increments a local counter
	while(1)
	 {
		DBSTREAM2(cprogress << "Test Task3 Running\n");
		TaskSwitch();
	 }
}

//=============================================================================

#if TEST_SIGNAL

#define TEST_SIG 1

void
TestTask4(void)
{
	// this task waits for a signal
	while(1)
	 {
		DBSTREAM2(cprogress << "Test Task4 Waiting for Signal\n");
		SignalWait(TEST_SIG);
		DBSTREAM2(cprogress << "Test Task4 Got Signal\n");
		TaskSwitch();
	 }
}

#endif

//=============================================================================

#if TEST_MESSAGE

void
TestTask5(void)
{
	int32 testMess;
	IMessagePort testPort;
	testPort = MessagePortNew("testPort");
	assert(testPort);
	PublicMessagePortListAdd(testPort);
	MessagePortAttachTask(testPort, GetCurrentTask());

	// this task waits for messages
	while(1)
	 {
		DBSTREAM2(cprogress << "Test Task5 Waiting for Message\n");
		MessagePortWait(testPort);
		MessagePortGet(testPort,&testMess);
		DBSTREAM2(cprogress << "Test Task5 Got Message " << testMess << endl);
		if(testMess == 0)					// message of 0 means terminate task
		 {
			PublicMessagePortListRemove(testPort);
			MessagePortDelete(testPort);
			TaskKill(GetCurrentTask());
		 }
		TaskSwitch();
	 }
}

#endif

//=============================================================================

void
TaskerTest(void)
{
	int counter = 0;
	ITask tempTask;
	ITask sigTask;
	ITask messageTask;
	IMessagePort testPort;
	tempTask = GetCurrentTask();
	VALIDATETASK(tempTask);

	DBSTREAM2(cprogress << "testing tasker:"<< endl);

	TaskSetPriority(GetCurrentTask(),1);

#if TEST_SIGNAL
	SignalTest();
	sigTask = CreateTask(&TestTask4,10,0);		// create task to send signals to
#endif

#if TEST_MESSAGE
	MessageTest();
	messageTask = CreateTask(&TestTask5,10,0);		// create task to send messages to
	TaskSwitch();									// give task 5 time to create port
	testPort = PublicMessagePortFind("testPort");
	assert(testPort);
#endif

	while(1)
	 {
		counter++;
		DBSTREAM2(cprogress << "Test Task Running\n");
		if(counter == 5)
		 {
			DBSTREAM2(cprogress << "Creating Task 2\n");
			CreateTask(&TestTask2,1,0);
		 }
		if(counter == 10)
		 {
			DBSTREAM2( cprogress << "Creating Task 3\n");
			tempTask = CreateTask(&TestTask3,1,0);
		 }
		if(counter == 30)
		 {
			DBSTREAM2( cprogress << "killing task 3\n");
			TaskKill(tempTask);
		 }
#if TEST_SIGNAL
		if(counter > 50 && counter < 60)
		 {
			DBSTREAM2( cprogress << "Sending signal\n");
			SignalSend(sigTask, TEST_SIG);			// send a signal to a task
		 }
#endif
#if TEST_MESSAGE
		if(counter > 60 && counter < 70)
		 {
			DBSTREAM2( cprogress << "Sending message\n");
			MessagePortPut(testPort, 0, counter);			// send a message to a task
		 }
#endif
		if(counter > 100)
		 {
#if TEST_MESSAGE
			DBSTREAM2( cprogress << "killing task 5\n");
			MessagePortPut(testPort, 0, 0);
			TaskSwitch();
#endif
			DBSTREAM2( cprogress << "killing task 4\n");
			TaskKill(sigTask);

			DBSTREAM2( cprogress << "Done testing tasker\n");
			return;
		 }
		TaskSwitch();
	 }
}
#endif

//=============================================================================

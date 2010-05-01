//=============================================================================
// _Signal.c: signal bit allocation/deallocation routines
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Dynamic allocation of single bit flags in a 32 bit long
//	History:
//			Created	12-12-94 04:02pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#define __sIGNAL_C
#include <hal/_signal.h>
#include <hal/item.h>
#include <hal/hal.h>

#include <cstdio>

//=============================================================================

halSignal
SignalAlloc(ITask iTask)
{
	halSignal sigAlloc,one;
	STask* task;
	VALIDATETASK(iTask);
	task = ITEMRETRIEVE(iTask,STask);
	VALIDATETASKPTR(task);

	sigAlloc = task->_sigAlloc;
	one = 1;
	while(one != 0x8000000)
	 {
		if(one & sigAlloc)
			one <<= 1;
		else
		 {
			task->_sigAlloc |= one;
			task->_sigRecieved &= ~one;
			task->_sigWait &= ~one;
			return(one);
		 }
	 }
#ifndef TEST_SIGNAL
	assert(0);				// out of signals
#endif
	return(0);
}

//=============================================================================

void
SignalFree(ITask iTask,halSignal sig)
{
	STask* task;
	VALIDATETASK(iTask);
	task = ITEMRETRIEVE(iTask,STask);
	VALIDATETASKPTR(task);
	VALIDATESIGNAL(sig);
	assert(task->_sigAlloc & sig);

	task->_sigAlloc &= ~sig;
}

//=============================================================================

void
SignalSend(ITask iTask, halSignal sig)			// send a signal to a task
{   STask* task;
	VALIDATETASK(iTask);
	task = ITEMRETRIEVE(iTask,STask);
	VALIDATETASKPTR(task);
	VALIDATESIGNAL(sig);
	task->_sigRecieved |= sig;
	if(task->_state == ETASKSTATE_WAITING && (task->_sigWait & sig))
	 {
		NodeRemove((SNode*)task);					// remove from ready list
		currentTask->_state = ETASKSTATE_READY;
		ListAddHead(&tasksReady, (SNode*)task);		// put in waiting list
		TaskSwitch();
	 }
}

//=============================================================================

void
SignalSendInterrupt(ITask iTask, halSignal sig)			// send a signal to a task
{   STask* task;
	VALIDATETASK(iTask);
	task = ITEMRETRIEVE(iTask,STask);
	VALIDATETASKPTR(task);
	VALIDATESIGNAL(sig);
	task->_sigRecieved |= sig;
	if(task->_state == ETASKSTATE_WAITING && (task->_sigWait & sig))
	 {
		NodeRemove((SNode*)task);					// remove from ready list
		currentTask->_state = ETASKSTATE_READY;
		ListAddHead(&tasksReady, (SNode*)task);		// put in waiting list
		// Note: no task switch here
	 }
}

//=============================================================================

// cause currently executing task to wait(lose CPU) until one of the signals in sent to it

halSignalMask
SignalWait(halSignalMask waitSignals)
{
	halSignalMask sigs;
	currentTask->_sigWait = waitSignals;
//	DisableInterrupts();


	if(!(currentTask->_sigWait & currentTask->_sigRecieved))		// if no received messages match messages we are waiting for
	 {
		NodeRemove((SNode*)currentTask);					// remove from ready list
		currentTask->_state = ETASKSTATE_WAITING;
		ListAddHead(&tasksWaiting, (SNode*)currentTask);	// put in waiting list
		TaskSwitch();
	 }
	sigs = currentTask->_sigWait & currentTask->_sigRecieved;
	currentTask->_sigRecieved &= ~sigs;

//	EnableInterrupts();
	return(sigs);
}

//=============================================================================
// test suite

#if TEST_SIGNAL

void
SignalTest(void)
{
	ITask iTestTask;
	STask* testTask;
	short i;
	halSignal s1,s2,s3;
	//ERR_DEBUG_LOG(ERR_HAL_PROGRESS, ("HAL Testing Signals\n"));

	iTestTask = GetCurrentTask();
	testTask = ITEMRETRIEVE(iTestTask,STask);

	assert(testTask->_sigAlloc == 0);
	assert(testTask->_sigWait == 0);
	assert(testTask->_sigRecieved == 0);

	s1 = SignalAlloc(iTestTask);
	assert(s1 == 1);
	assert(testTask->_sigAlloc == 1);
	s2 = SignalAlloc(iTestTask);
	assert(s2 == 2);
	assert(testTask->_sigAlloc == 3);
	s3 = SignalAlloc(iTestTask);
	assert(s3 == 4);
	assert(testTask->_sigAlloc == 7);
	SignalFree(iTestTask,s2);
	assert(testTask->_sigAlloc == 5);
	SignalFree(iTestTask,s1);
	assert(testTask->_sigAlloc == 4);
	SignalFree(iTestTask,s3);
	assert(testTask->_sigAlloc == 0);

	s1 = SignalAlloc(iTestTask);
	assert(s1 == 1);
	assert(testTask->_sigAlloc == 1);
	s2 = SignalAlloc(iTestTask);
	assert(s2 == 2);
	assert(testTask->_sigAlloc == 3);
	s3 = SignalAlloc(iTestTask);
	assert(s3 == 4);
	assert(testTask->_sigAlloc == 7);
	SignalFree(iTestTask,s1);
	assert(testTask->_sigAlloc == 6);
	SignalFree(iTestTask,s2);
	assert(testTask->_sigAlloc == 4);
	s1 = SignalAlloc(iTestTask);
	assert(s1 == 1);
	assert(testTask->_sigAlloc == 5);

	SignalFree(iTestTask,s3);
	assert(testTask->_sigAlloc == 1);
	SignalFree(iTestTask,s1);
	assert(testTask->_sigAlloc == 0);

	for(i=1;i<32;i++)
	 {
		s1 = SignalAlloc(iTestTask);
		assert(!(s1 == (1<<i)));
	 }
	s1 = SignalAlloc(iTestTask);
	assert(s1 == 0);							// out of signals

	// the sending and recieving of signals is tested in tasker.c
}

#endif

//=============================================================================

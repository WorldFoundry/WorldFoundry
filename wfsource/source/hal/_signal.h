//=============================================================================
// _Signal.h: signal bit allocation/deallocation routines for the tasker
//=============================================================================
// use only once insurance

#ifndef __sIGNAL_H
#define __sIGNAL_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Provides a simple mechanism to cause a task to get awakened,
//			used by the messaing system in the tasker
//	History:
//			Created	12-12-94 04:02pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>
#include <hal/_tasker.h>

//=============================================================================
// debugging macros

#if DO_ASSERTIONS
#define VALIDATESIGNAL(halSignal) \
	assert(halSignal);						/* zero is an invalid signal */\
	{\
		int32 tempSignal = halSignal;\
		short bitCount = 0;\
		short counter = 32;\
		while(counter--)\
		 {\
			if(tempSignal & 1)\
				bitCount++;\
			tempSignal >>= 1;\
		 }\
	assert(bitCount == 1);\
	}
#else
#define VALIDATESIGNAL(halSignal)
#endif

//=============================================================================

typedef int32 halSignal;
typedef int32 halSignalMask;				// more than one signal or'ed together
#define SIGNAL_INVALID 0

halSignal
SignalAlloc(ITask task);

void
SignalFree(ITask task, halSignal sig);

halSignalMask                         					// global, operates on currently executing task
SignalWait(halSignalMask waitSignals);

void
SignalSend(ITask task, halSignal sig);			// send a signal to a task

void
SignalSendInterrupt(ITask task, halSignal sig);			// send a signal to a task

//=============================================================================
// test suite

#if TEST_SIGNAL
void
SignalTest(void);
#endif

//=============================================================================
#endif
//=============================================================================

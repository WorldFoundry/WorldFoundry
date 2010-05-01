//=============================================================================
// HalBase.h:
//=============================================================================
// use only once insurance

#ifndef	_hALBASE_H
#define	_hALBASE_H

//=============================================================================
//  Documentation:
//=============================================================================

//	Abstract:
//		this file is included by all the other PIGS C header files, anything
//		which must be done globaly should be done here

//	History:
//			Created	9-12-92 03:26pm Kevin T. Seghetti
//	Class Hierarchy:
//			none

//	Dependancies:
//			none

//	Restrictions:

//	Example:

//=============================================================================

#include <pigsys/pigsys.hp>
#include <memory/lmalloc.hp>
#include <memory/dmalloc.hp>

#if !defined( HAL_MAX_TASKS )
#define HAL_MAX_TASKS	30
#endif

#if !defined( HAL_MAX_MESSAGES )
#define HAL_MAX_MESSAGES	200
#endif

#if !defined( HAL_MAX_PORTS )
#define HAL_MAX_PORTS	40
#endif

#if !defined( HAL_MAX_TIMER_SUBSCRIBERS )
#define HAL_MAX_TIMER_SUBSCRIBERS	40
#endif

#if !defined( NO_CD )
#define NO_CD	1
#endif

#if !defined( ITEM_PRINT )
#define ITEM_PRINT	0
#endif

#if !defined( HAL_INTERRUPTS )
#define HAL_INTERRUPTS	0
#endif

#if DO_TEST_CODE
#define TEST_LIST	1
#define TEST_JOYSTICK	1
#define TEST_SIGNAL	1
#define TEST_MESSAGE	1
#define TEST_GENERAL	1
#define TEST_ITEM	1
#define TEST_MEMPOOL	1
#define TEST_DISKFILE	1
#if defined( DO_MULTITASKING )
#	define TEST_TASKER	1
//#	define TEST_TIMER	1
#	define TEST_TIMER	0
#endif
#else
#define TEST_LIST	0
#define TEST_JOYSTICK	0
#define TEST_TASKER	0
#define TEST_SIGNAL	0
#define TEST_MESSAGE	0
#define TEST_GENERAL	0
#define TEST_TIMER	0
#define TEST_ITEM	0
#define TEST_MEMPOOL	0
#define TEST_DISKFILE	0
#endif

typedef void (voidFunction)(void);

//=============================================================================
// restrictions, maximums, etc

#define STRINGMAXLEN 128

// UNIMPLEMENTED can be used to flag any piece of code which should not execute
// with debugging off, it becomes undefined, and therefore an error
#if DO_ASSERTIONS
#define UNIMPLEMENTED() assert(0)
#endif

//==============================================================================
// global lmalloc

extern int32 cbHalLmalloc;

extern LMalloc* _HALLmalloc;
#define HALLmalloc (*_HALLmalloc)

extern int32 cbHalScratchLmalloc;

extern LMalloc* _HALScratchLmalloc;
#define HALScratchLmalloc (*_HALScratchLmalloc)

#define HAL_DMALLOC_SIZE 1000

extern DMalloc* _HALDmalloc;
#define HALDmalloc (*_HALDmalloc)

//=============================================================================
// this macro is machine specific

// at least check for NULL
#define VALIDATEPTR(ptr) assert(ptr)

#define VALIDATESTRING(str) \
{\
	VALIDATEPTR(str); \
	assert(strlen(str) < STRINGMAXLEN); \
}

//=============================================================================
// suggested task priorities of PIGS/HAL components
// I can't think of good names for these, if you can, let me know

enum
{
	PRIORITY_SYSTEM_CRITICAL=2000,	// not allowed for clients, only system code
	PRIORITY_CRITICAL=1000,			// interrupt code
	PRIORITY_HIGH=600,          	// VERY time dependent tasks go here
	PRIORITY_MEDIUM=400,        	// time dependent tasks go here (maybe sound driver)
	PRIORITY_ATTENTIVE=200,			// just any task which should respond immediatly to messages from normal tasks
	PRIORITY_NORMAL=0,				// most game tasks should be here
	PRIORITY_BACKGROUND=-200,		// only runs when everyone else is waiting
	PRIORITY_LOWEST = -1000			// only runs when even background tasks are waiting
};

//=============================================================================
#endif
//=============================================================================

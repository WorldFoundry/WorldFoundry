//==============================================================================
// hal.h: PIGS main entry point, this header includes all others
//==============================================================================
// use only once insurance

#ifndef _hAL_H
#define _hAL_H

//==============================================================================
// Documentation:
//=============================================================================

//	Abstract:
//		PIGS HAL main include file interface, includes all hal header files
//	History:
//			Created	01-11-95 05:12pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//		All of PIGS HAL
//	Restrictions:

//	Example:
//==============================================================================
// all public HAL header files

#include <cpplib/stdstrm.hp>

#include <hal/halbase.h>
#if defined(DO_MULTITASKING)
#include <hal/tasker.h>
#include <hal/timer.h>
#endif
#include <hal/message.h>
#include <hal/general.h>
//#include <hal/time.hp>
#include <hal/item.h>
#include <hal/sjoystic.h>
#include <hal/platform.h>

//==============================================================================
// user must provide this function, think of it as "main"

void PIGSMain(int argc, char* argv[]);						// prototype for user start function
bool PIGSUserAborted(void);				// returns true if user has aborted game
void PIGSExit(void);						// call to shut down PIGS, doesn't return
void HALStart(int argc, char** argv, int maxTasks,int maxMessages, int maxPorts);

//==============================================================================

class BeforeMain
{
public:
	BeforeMain();

};

//==============================================================================

void
FatalError( const char* string );

//==============================================================================

#if DO_DEBUGGING_INFO
void breakpoint();
#endif

//==============================================================================
// these are the error log bits for the HAL

#define	ERR_HAL_LOGMASK	0x000000F0ul		// all hal bits
#define	ERR_HAL_WARNING	0x00000010ul		// could be a bug here
#define	ERR_HAL_ERROR	0x00000020ul		// definitly a bug here
#define	ERR_HAL_PROGRESS	0x00000040ul 	// lots of call progress

//==============================================================================
#endif
//==============================================================================

//=============================================================================
// _platfor.h: platform specific code interface
//=============================================================================
// use only once insurance

#ifndef _pLATFORM_H
#define _pLATFORM_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//		header file for prototypes of platform specific calls
//	History:
//			Created	03-07-95 11:43am Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//	Restrictions:

//	Example:

//=============================================================================

#include <hal/hal.h>

//=============================================================================

void
_PlatformSpecificInit(int argc, char** argv, int maxTasks,int maxMessages, int maxPorts);

void
_PlatformSpecificUnInit(void);

//=============================================================================
#endif
//=============================================================================

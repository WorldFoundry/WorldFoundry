//=============================================================================
// testhal.h: test PIGS sub-systems
//=============================================================================
// use only once insurance

#ifndef _tESTHAL_H
#define _tESTHAL_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//			Test code for PIGS
//	History:
//			Created	10-24-94 06:26pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//			none

//	Restrictions:
//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>

//=============================================================================
// debugging macros

//=============================================================================
// function prototypes

// will test all pigs systems which have the TEST macro set to 1(i.e. TEST_LIST)
// see projdefs.mk for a complete list


#if DO_TEST_CODE
void
TestHAL(void);
#endif

//=============================================================================
#endif
//=============================================================================

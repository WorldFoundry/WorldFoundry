//=============================================================================
// callback.c: psx specific callbacks, in separate file so it won't get optimized
//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:

//	History:
//			Created 03-29-97 03:58pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#define _CALLBACK_C

#include <hal/hal.h>			// includes everything

extern "C" {
#	include <r3000.h>
#	include <asm.h>
#	include <kernel.h>
#	include <libetc.h>
#	include <stdio.h>
#	include <libsn.h>

#	include <missing.h>
#	include <libsn.h>
	};

//=============================================================================

//extern u_long _PigsRootCounter2;

//=============================================================================

//long _PigsRootCounterCallBack( void )
//{
//	++_PigsRootCounter2;
//	return(0);
//}

//=============================================================================

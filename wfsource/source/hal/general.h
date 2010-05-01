//=============================================================================
// general.h: simple boring routines header for PIGS
//=============================================================================
// use only once insurance

#ifndef _gENERAL_H
#define _gENERAL_H

//=============================================================================
// Documentation:
//=============================================================================
//	Abstract:
//		Simple string manipulation, etc
//	History:
//			Created	12-15-94 12:32pm Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:

//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#include <hal/halbase.h>

//=============================================================================
// strncpy sucks, and has several dangerous behaviors, so I wrote my own

// will copy up to count-1 characters from source to dest, and then zero
//  terminate it(count should be the total # of destination bytes availible,
// including the zero termination)

void
StringCopyCount(char* dest,const char* source,size_t count);

//=============================================================================
#endif
//=============================================================================

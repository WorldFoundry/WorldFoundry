//============================================================================
// hdump.hp: formated hex and ascii data dumper
// Copyright(c) 1994/5 Cave Logic Studios
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			formated hex and ascii data dumper
	History:

	Class Hierarchy:
			none

	Dependancies:
			iostream (cout)
	Restrictions:
	Example:
*/
//============================================================================
// use only once insurance
#ifndef _hDUMP_HP
#define _hDUMP_HP

#include "global.hpp"
#include <iostream.h>

//#include <pclib/general.hp>

//============================================================================

void
HDump(void* buffer, ulong bufferSize, int indent=0,char* indentString = ">", ostream& out=cout);

//============================================================================
#endif
//============================================================================

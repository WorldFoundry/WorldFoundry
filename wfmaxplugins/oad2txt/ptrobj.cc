//============================================================================
// Ptrobj.cc:
// Copyright(c) 1996 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			A PtrObj encapsulates a pointer to some other class, for use with
			STL containers.
	History:
			Created	08/12/96 16:18 by Phil Torre

	Class Hierarchy:
			none

	Dependancies:

	Restrictions:

	Example:
*/
//============================================================================

#include "global.hpp"
#include "ptrobj.hpp"


PtrObj::PtrObj(T* thePointer)
{
	_thePointer = thePointer;
}

PtrObj::PtrObj()
{
	_thePointer = NULL;
}

PtrObj::~PtrObj()
{

}

PtrObj& PtrObj::operator=(const PtrObj& other)
{
	_thePointer = other._thePointer;
	return (*this);
}

bool PtrObj::operator==(const PtrObj& other)
{
	return (_thePointer == other._thePointer);
}

bool operator>(const PtrObj& other)
{
	return (_thePointer > other._thePointer);
}

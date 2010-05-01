//============================================================================
// salloc.cc: stacked memory allocator (variable sized allocations, must be used
// in fifo order (asserts otherwise)
//============================================================================
// Documentation:

//	Abstract:
//			currently used by the tasker for stack managment

//	History:
//			Created	8/24/96 14:51 Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//			halbase.h
//			_list.h

//	Restrictions:


// tasker functions should assert that tasker is running

//============================================================================
// dependencies

#include <hal/hal.h>
#include <hal/salloc.hp>

//============================================================================

SAlloc::SAlloc(void* memory,size_t size)
{
	VALIDATEPTR(memory);
	assert(size);
	_memoryBase = (char*)memory;
	_freeMemory = _memoryBase;
	_memorySize = size;
#if DO_ASSERTIONS
	_allocIndex = -1;
#endif
}

//============================================================================

SAlloc::~SAlloc(void)
{
	assert(_freeMemory == _memoryBase);
#if DO_ASSERTIONS
	assert(_allocIndex == -1);
#endif
#pragma message ("kts: who should free SAllocs memory?")
}

//============================================================================

void*
SAlloc::Allocate(size_t size)
{
#if DO_ASSERTIONS
	size_t left = _memorySize - (_freeMemory - _memoryBase);
#endif
	AssertMsg(size < left,"out of stack space, asked for " << size << ", only have " << left);			// will it fit?

#if DO_ASSERTIONS
	_allocIndex++;
	_allocPtrs[_allocIndex] = _freeMemory;
#endif

	void* retVal = (void*)_freeMemory;
	_freeMemory += size;
	return(retVal);
}

//============================================================================

void
SAlloc::Free(void* memory)
{
#if DO_ASSERTIONS
	assert(_allocIndex >= 0);
	assert(memory == _allocPtrs[_allocIndex]);
	_allocIndex--;
#endif
	_freeMemory = (char*)memory;
}

//============================================================================

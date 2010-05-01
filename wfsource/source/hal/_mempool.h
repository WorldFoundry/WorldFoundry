//==============================================================================
// _mempool.h
//==============================================================================
// use only once insurance

#ifndef _mEMpOOL_H
#define _mEMpOOL_H

//==============================================================================

#include <cstdio>

//==============================================================================

struct _MemPoolFreeEntry
{
	_MemPoolFreeEntry* _next;
};

//==============================================================================

typedef struct _MemPool
{
	size_t _size;					// size of each entry in pool
	char* _buffer;					// actual pool
	_MemPoolFreeEntry _firstFree;	// contains pointer to first free node
	Memory* _parentMemory;			    // parent memory was allocated from

#if DO_ASSERTIONS
	int _maxEntries;				// maximum # of entries in pool
	int _currentEntries;			// current # of allocated entries in pool
#endif
} SMemPool;


//==============================================================================
// debugging macros

#if DO_ASSERTIONS
INLINE void
VALIDATEMEMPOOL(SMemPool* memPool)
 {
	VALIDATEPTR(memPool);
	VALIDATEPTR(memPool->_buffer);
	assert(memPool->_maxEntries > 0);
	assert(memPool->_currentEntries >= 0);
	assert(memPool->_size > 0);
	assert(memPool->_maxEntries > memPool->_currentEntries);
	_MemPoolFreeEntry* fe = &memPool->_firstFree;
	int count = 0;
	while(fe->_next)
	 {
		assert(ValidPtr(fe->_next));
		AssertMsg(((char*)fe->_next) >= memPool->_buffer,
			"should be between " << (void*)memPool->_buffer << " and " << (void*)(memPool->_buffer+(memPool->_maxEntries*memPool->_size)) << ", is " << fe->_next);
		assert(((char*)fe->_next) < (memPool->_buffer+(memPool->_maxEntries*memPool->_size)));
		fe = fe->_next;
		AssertMsg(count < memPool->_maxEntries, "mempool list must be looped");
		count++;
	 }
 }
#else
#define VALIDATEMEMPOOL(memPool)
#endif

//==============================================================================

SMemPool*
MemPoolConstruct(size_t size, int entries, Memory& memory);

void*
MemPoolDestruct(SMemPool* smem);

void*
MemPoolAllocate(SMemPool* mPool, size_t size);

void*
MemPoolFree(SMemPool* mPool, void* mem);

int
MemPoolEntries(SMemPool* mPool);

#if TEST_MEMPOOL
void
MemPoolTest(void);
#endif

//==============================================================================
#endif
//==============================================================================

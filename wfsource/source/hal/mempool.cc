//============================================================================
// MemPool.cc
//============================================================================
// static memory array allocator
//
// since the memory pool code is private to the HAL, I don't bother using the
// item system to track it
//
//============================================================================
// The memory pool code implements an array of fixed size entries which may be
// allocated and freed.  The implementation maintains a forward linked list of
// free nodes in the free nodes themselves, thus wasting no space.  Both allocation
// and freeing a node is constant time, and fast.
//============================================================================

#include <hal/hal.h>
#include <hal/_mempool.h>

#define MEMPOOL_REALTRACKING 1
#define MEMPOOL_TRASHMEMORY DO_ASSERTIONS
#define MEMPOOL_FILLVALUE 0xa5

//==============================================================================
// create an entirly new memory pool

SMemPool*
MemPoolConstruct(size_t size,int entries,Memory& memory)
{
	assert(size);
	AssertMsg((size % 4) == 0,"MemPool size must be long-word alligned, size was " << size);								// must be long-word aligned
	assert(entries);
	assert(size >= sizeof(_MemPoolFreeEntry));				// make sure our free entry struct will fit

	SMemPool* memPool;
	memPool = new (memory) SMemPool;
	assert(memPool);
	AssertMemoryAllocation(memPool);
	assert(((long)memPool % 4) == 0);
#if DO_ASSERTIONS
	memPool->_size = size;
	memPool->_maxEntries = entries;
	memPool->_currentEntries = 0;
#endif
	assert(size*entries);


	memPool->_parentMemory = &memory;
	memPool->_buffer = new (memory) char[size*entries];
	assert(memPool->_buffer);
	AssertMemoryAllocation(memPool->_buffer);

	_MemPoolFreeEntry* freeEntry = &memPool->_firstFree;
	char* nextFree = memPool->_buffer;
	for(int entry=0;entry<entries;entry++)
	 {
		freeEntry->_next = (_MemPoolFreeEntry*)nextFree;
		freeEntry = freeEntry->_next;
		nextFree += size;
	 }
	freeEntry->_next = NULL;

	VALIDATEMEMPOOL(memPool);
	DBSTREAM2( cprogress << "Creating a memory pool at address " << memPool << " with " << entries << " entries, " << size << " bytes each\n");
	return(memPool);
}

//==============================================================================
// destroy an entire memory pool

void*
MemPoolDestruct(SMemPool* memPool)
{
	VALIDATEMEMPOOL(memPool);
	DBSTREAM2(cprogress <<  "Destroying a memory pool at address " << memPool ASSERTIONS( << " with " << memPool->_maxEntries << " entries" ) << ", " << memPool->_size << " bytes each\n");
	assert(memPool->_currentEntries == 0);		// if allocations remain, there must be a problem
	memPool->_parentMemory->Free(memPool->_buffer);
	memPool->_parentMemory->Free(memPool);
	return(NULL);
}

//==============================================================================
// allocate one entry out of an existing memory pool

void*
MemPoolAllocate(SMemPool* memPool, size_t size)
{
	(void)size;
//	printf("mempool = %x\n",memPool);
	VALIDATEMEMPOOL(memPool);			// input validation
	assert(size == memPool->_size);
	AssertMsg( memPool->_currentEntries < memPool->_maxEntries,"memPool->_currentEntries=" << memPool->_currentEntries << " memPool->_maxEntries=" << memPool->_maxEntries );

#if MEMPOOL_REALTRACKING
	assert(memPool->_firstFree._next);
	if(memPool->_firstFree._next)
	 {
#if  DO_ASSERTIONS
		memPool->_currentEntries++;
#endif
		void* mem = (void*)memPool->_firstFree._next;
		memPool->_firstFree._next = memPool->_firstFree._next->_next;
		DBSTREAM5(cprogress <<  "Allocating an entry from Memory Pool " << memPool << " at address " << mem ASSERTIONS( << " for a total of " <<  memPool->_currentEntries << " entries") << endl;)
		return(mem);
	 }
	else
		return(0);					// out of entries
#else
#if  DO_ASSERTIONS
	memPool->_currentEntries++;
#endif
	assert(0);
	void* mem = malloc(memPool->_size);
	assert(mem);
	AssertMemoryAllocation(mem);
	DBSTREAM5(cprogress <<  "Allocating an entry from Memory Pool " << memPool << " at address " << mem ASSERTIONS( << "  for a total of " << memPool->_currentEntries << " entries") << endl;))
	return(mem);
#endif
}

//==============================================================================
// free one entry in an existing memory pool

void*
MemPoolFree(SMemPool* memPool, void* mem)
{
	VALIDATEMEMPOOL(memPool);
	assert(ValidPtr(mem));
#if  DO_ASSERTIONS
	memPool->_currentEntries--;
#endif
	DBSTREAM5(cprogress <<  "Freeing an entry from Memory Pool " << memPool << " at address " << mem ASSERTIONS( << " leaving " << memPool->_currentEntries << " entries") << endl; )
	assert(memPool->_currentEntries >= 0);

#if MEMPOOL_REALTRACKING
	assert(((char*)mem) >= memPool->_buffer);
	assert(((char*)mem) < (memPool->_buffer+(memPool->_maxEntries*memPool->_size)));

#if  DO_ASSERTIONS
	{
		_MemPoolFreeEntry* fe = &memPool->_firstFree;
		int count = 0;
		while(fe->_next)
		 {
			assert(mem != fe->_next);
			fe = fe->_next;
		 }
	}
#endif

#if MEMPOOL_TRASHMEMORY
	memset((unsigned char*)mem,MEMPOOL_FILLVALUE,memPool->_size);
#endif

	_MemPoolFreeEntry* fe = (_MemPoolFreeEntry*)mem;
	fe->_next = memPool->_firstFree._next;
	memPool->_firstFree._next = fe;
	VALIDATEMEMPOOL(memPool);
#else
	free(mem);
#endif
	return(NULL);
}

//==============================================================================

#if  DO_ASSERTIONS
int
MemPoolEntries(SMemPool* memPool)
{
	VALIDATEMEMPOOL(memPool);
	return(memPool->_currentEntries);
}
#endif

//==============================================================================

#if TEST_MEMPOOL

void
MemPoolTest(void)
{
#define MEMPOOL_SIZE 24
#define MEMPOOL_ENTRIES 10
	SMemPool* testMemPool = MemPoolConstruct(MEMPOOL_SIZE,MEMPOOL_ENTRIES,HALLmalloc);
	assert(testMemPool);
	VALIDATEMEMPOOL(testMemPool);

	void* test1 = MemPoolAllocate(testMemPool,MEMPOOL_SIZE);
	assert(ValidPtr(test1));
	VALIDATEMEMPOOL(testMemPool);
	assert(MemPoolEntries(testMemPool) == 1);
	void* test2 = MemPoolAllocate(testMemPool,MEMPOOL_SIZE);
	assert(ValidPtr(test2));
	assert(test2 != test1);
	VALIDATEMEMPOOL(testMemPool);
	assert(MemPoolEntries(testMemPool) == 2);
	void* test3 = MemPoolAllocate(testMemPool,MEMPOOL_SIZE);
	assert(ValidPtr(test3));
	assert(test1 != test3);
	assert(test2 != test3);
	VALIDATEMEMPOOL(testMemPool);
	assert(MemPoolEntries(testMemPool) == 3);

	MemPoolFree(testMemPool,test2);
	assert(MemPoolEntries(testMemPool) == 2);
	void* test4 = MemPoolAllocate(testMemPool,MEMPOOL_SIZE);
	assert(ValidPtr(test4));
	assert(MemPoolEntries(testMemPool) == 3);
	assert(test4 == test2);					// rely on implementation to insure no leakage


	MemPoolFree(testMemPool,test1);
	assert(MemPoolEntries(testMemPool) == 2);
	MemPoolFree(testMemPool,test3);
	assert(MemPoolEntries(testMemPool) == 1);
	MemPoolFree(testMemPool,test4);
	assert(MemPoolEntries(testMemPool) == 0);

	MemPoolDestruct(testMemPool);
}

#endif

//==============================================================================

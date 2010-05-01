//============================================================================
// mempool.cc
// Copyright ( c ) 1995,1996,1997,1999,2000,2003 World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

//============================================================================
// C++ version of static memory array allocator
//
//============================================================================
// The memory pool code implements an array of fixed size entries which may be
// allocated and freed.  The implementation maintains a forward linked list of
// free nodes in the free nodes themselves, thus wasting no space.  Both allocation
// and freeing a node is constant time, and fast.
//============================================================================

#include <hal/hal.h>
#include <memory/mempool.hp>

#define MEMPOOL_REALTRACKING 1
#define MEMPOOL_TRASHMEMORY DO_ASSERTIONS
#define MEMPOOL_FILLVALUE 0xa5

//==============================================================================
// create an entirly new memory pool

MemPool::MemPool(size_t size,int entries,Memory& memory MEMORY_NAMED( COMMA const char* name ))
	: Memory( MEMORY_NAMED( name ) )
{
	assert(size);
	AssertMsg((size % 4) == 0,"MemPool size must be long-word alligned, size was " << size);								// must be long-word aligned
	assert(entries);
	assert(size >= sizeof(MemPoolFreeEntry));				// make sure our free entry struct will fit
	assert(size*entries);

	_memory = &memory;
	_buffer = new (memory) char[size*entries];
	assert(_buffer);
	AssertMemoryAllocation(_buffer);

#if DO_ASSERTIONS
#endif
	_currentEntries = 0;
	_size = size;
	_maxEntries = entries;
	Clear();
	Validate();
	DBSTREAM2( cprogress << "Creating a memory pool at address " << this << " with " << entries << " entries, " << size << " bytes each\n");
}

//==============================================================================
// destroy an entire memory pool

MemPool::~MemPool()
{
	Validate();
	DBSTREAM2(cprogress << "Destroying a memory pool at address " << this ASSERTIONS( << " with " <<  _maxEntries << " entries, " << _size << " bytes each" ) << std::endl;)
	assert(_currentEntries == 0);		// if allocations remain, there must be a problem
	_memory->Free(_buffer);
}

//==============================================================================
// allocate one entry out of an existing memory pool

void*
MemPool::Allocate(size_t size ASSERTIONS( COMMA const char* file COMMA int line) ) 	// get a single object from the pool
{
	Validate();			// input validation
	assert(size == _size);
	AssertMsg( _currentEntries < _maxEntries,"_currentEntries=" << _currentEntries << " _maxEntries=" << _maxEntries );

#if MEMPOOL_REALTRACKING
	assert(_firstFree._next);
	if(_firstFree._next)
	 {
//#if DO_ASSERTIONS
		_currentEntries++;
//#endif
		void* mem = (void*)_firstFree._next;
		_firstFree._next = _firstFree._next->_next;
		DBSTREAM5(cprogress << "Allocating an entry from Memory Pool " << this << " at address " << mem ASSERTIONS( << " for a total of " <<  _currentEntries << " entries") << std::endl;)
		return(mem);
	 }
	else
		return(0);					// out of entries
#else					// ! MEMPOOL_REALTRACKING
//#if DO_ASSERTIONS
	_currentEntries++;
//#endif
#error !!!
	void* mem = malloc(_size);
	assert(mem);
	AssertMemoryAllocation(mem);
	DBSTREAM5(cprogress << "Allocating an entry from Memory Pool " << this << " at address " << mem << " for a total of " <<  _currentEntries << " entries\n");
	return(mem);
#endif
}

//==============================================================================
// free one entry in an existing memory pool

void
MemPool::Free(const void* mem)
{
	Validate();
	assert(ValidPtr(mem));
//#if DO_ASSERTIONS
	_currentEntries--;
//#endif
	DBSTREAM5(cprogress << "Freeing an entry from Memory Pool " << this << " at address " << mem ASSERTIONS( << " leaving " << _currentEntries << " entries") << std::endl; )
	assert(_currentEntries >= 0);

#if MEMPOOL_REALTRACKING
	assert(((char*)mem) >= _buffer);
	assert(((char*)mem) < (_buffer+(_maxEntries*_size)));

#if DO_ASSERTIONS
	{
		MemPoolFreeEntry* fe = &_firstFree;
//		int count = 0;
		while(fe->_next)
		 {
			assert(mem != fe->_next);
			fe = fe->_next;
		 }
	}
#endif

#if MEMPOOL_TRASHMEMORY
	memset((unsigned char*)mem,MEMPOOL_FILLVALUE,_size);
#endif

	MemPoolFreeEntry* fe = (MemPoolFreeEntry*)mem;
	fe->_next = _firstFree._next;
	_firstFree._next = fe;
	Validate();
#else
	free(mem);
#endif
//	return(NULL);
}

//==============================================================================

#if DO_ASSERTIONS
int
MemPool::Entries()
{
	Validate();
	return(_currentEntries);
}
#endif

//==============================================================================

#if TEST_MEMPOOL

void
CPPMemPoolTest(void)
{
#define MEMPOOL_SIZE 24
#define MEMPOOL_ENTRIES 10
	MemPool* testMemPool = new MemPool(MEMPOOL_SIZE,MEMPOOL_ENTRIES,HALLmalloc MEMORY_NAMED(COMMA "Test MemPool"));
	assert(testMemPool);
	testMemPool->Validate();

	void* test1 = testMemPool->Allocate(MEMPOOL_SIZE ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(test1));
	testMemPool->Validate();
	assert(testMemPool->Entries() == 1);
	void* test2 = testMemPool->Allocate(MEMPOOL_SIZE  ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(test2));
	assert(test2 != test1);
	testMemPool->Validate();
	assert(testMemPool->Entries() == 2);
	void* test3 = testMemPool->Allocate(MEMPOOL_SIZE  ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(test3));
	assert(test1 != test3);
	assert(test2 != test3);
	testMemPool->Validate();
	assert(testMemPool->Entries() == 3);

	testMemPool->Free(test2);
	assert(testMemPool->Entries() == 2);
	void* test4 = testMemPool->Allocate(MEMPOOL_SIZE  ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(test4));
	assert(testMemPool->Entries() == 3);
	assert(test4 == test2);					// rely on implementation to insure no leakage


	testMemPool->Free(test1);
	assert(testMemPool->Entries() == 2);
	testMemPool->Free(test3);
	assert(testMemPool->Entries() == 1);
	testMemPool->Free(test4);
	assert(testMemPool->Entries() == 0);

	delete testMemPool;
}

#endif

//==============================================================================

#if DO_ASSERTIONS

void
MemPool::_Validate() const
{
	VALIDATEPTR(this);
	VALIDATEPTR(_buffer);
	assert(_maxEntries > 0);
	assert(_currentEntries >= 0);
	assert(_size > 0);
	assert(_maxEntries > _currentEntries);
	const MemPoolFreeEntry* fe = &_firstFree;
	int count = 0;
	while(fe->_next)
	 {
		assert(ValidPtr(fe->_next));
		AssertMsg(((char*)fe->_next) >= _buffer,
			"should be between " << (void*)_buffer << " and " << (void*)(_buffer+(_maxEntries*_size)) << ", is " << fe->_next);
		assert(((char*)fe->_next) < (_buffer+(_maxEntries*_size)));
		fe = fe->_next;
		AssertMsg(count < _maxEntries, "mempool list must be looped");
		count++;
	 }
}
#endif

//==============================================================================

#if DO_IOSTREAMS

std::ostream&
MemPool::_Print(std::ostream& out) const
{
	out << "MemPool Dump: [named " << Name() << ']' << std::endl;
	return out;
}

#endif

//==============================================================================

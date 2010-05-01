//==============================================================================
// DMalloc.cc
// Copyright ( c ) 1997,1998,1999,2000,2001 World Foundry Group.  
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

//==============================================================================
// Description:
// Dynamic memory allocation class
//============================================================================
// notes: size on a FreeChunk is the total size of the chunk, including the area
// occupied by the FreeChunk itself
// Size on an AllocatedChunk is the size of the requested memory, it does NOT
// include the size of the AllocatedChunk

#define _DMalloc_CC
#include <memory/dmalloc.hp>
#include <cpplib/stdstrm.hp>
#include <streams/dbstrm.hp>
#include <hal/hal.h>
#include <cpplib/libstrm.hp>

//=============================================================================

#if DO_ASSERTIONS

void
DMalloc::_Validate() const
{
	Memory::Validate();

	ValidatePtr(_memory);
	ValidatePtr(_endMemory);
	if(_parentMemory)
		ValidatePtr(_parentMemory);

	FreeChunk* chunk = _firstChunk;
	while(chunk)
	{
		ValidatePtr(chunk);
		RangeCheck(_memory,(char*)chunk,_endMemory);
		chunk->Validate();
		if(chunk->Next())
			RangeCheck(_memory,(char*)chunk->Next(),_endMemory);
		chunk = chunk->Next();
	}
}

#endif	// DO_ASSERTIONS

//=============================================================================

#if DO_IOSTREAMS

std::ostream&
DMalloc::_Print(std::ostream& out) const
{
	Validate();
	out << "DMalloc Dump: [named " << Name() << ']' << std::endl;
	out << "_flags = " << _flags;
	out << ", _firstChuunk = " << (void*)_firstChunk << std::endl;
	out << "_memory = " << (void*)_memory;
	out << ", _endMemory = " << (void*)_endMemory;
	out << ", _parentMemory = " << (void*)_parentMemory << std::endl;

	// now walk free list
	FreeChunk* chunk = _firstChunk;
	while(chunk)
	{
		chunk->_Print(out);
		chunk = chunk->Next();
	}
	return out;
}

#endif

//=============================================================================

DMalloc::DMalloc(Memory& memory, size_t size MEMORY_NAMED ( COMMA const char* name ) )
	// allocates from another DMalloc memory pool
	: Memory( MEMORY_NAMED( name ) )
{
	assert(size);
	memory.Validate();
	_memory = (char*)memory.Allocate(size ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(_memory));
	AssertMemoryAllocation(_memory);
	//DBSTREAM1( printf("DMalloc::DMalloc: allocated %d bytes from memory at address %p\n",size,_memory); )
	//printf("New DMalloc ");
	//MEMORY_NAMED(printf("named %s ",name);)
	//printf(" allocated %d bytes from memory ",size);
	//MEMORY_NAMED(printf(" named %s ",memory.Name());)
	//printf(" at address %p\n",_memory);
	_endMemory = _memory + size;
	_flags = FLAG_MEMORY_OWNED;
	_parentMemory = &memory;
	assert(ValidPtr(_parentMemory));
	_firstChunk = new (_memory) FreeChunk(NULL,_endMemory-_memory);
	Validate();
}

//=============================================================================

#if 0
DMalloc::DMalloc(size_t size MEMORY_NAMED( COMMA const char* name ) ) : Memory( MEMORY_NAMED( name ) )
{
	assert(size);
	_memory = (char*)malloc(size);
	assert(ValidPtr(_memory));
	AssertMemoryAllocation(_memory);
	_endMemory = _memory + size;
	_flags = FLAG_MEMORY_OWNED;
	_parentMemory = NULL;
	_firstChunk = new (_memory) FreeChunk(NULL,_endMemory-_memory);
	Validate();
	//DBSTREAM1( printf("DMalloc::DMaloc constructed from heap at address %p with a size of %d\n",_memory,size); )
	//printf("DMalloc ");
	//DBSTREAM1( MEMORY_NAMED(printf(" named %s ",name); ))
	//printf("constructed from heap at addr %p, size = %d\n",_memory,size);
}
#endif

//=============================================================================

#if 0
DMalloc::DMalloc(void* memory, size_t size MEMORY_NAMED( COMMA const char* name ) )
	: Memory( MEMORY_NAMED( name ) )
{
	assert(0);		// until not using malloc, can't do this
}
#endif

//=============================================================================

DMalloc::~DMalloc()
{
	Validate();
	if(_flags & FLAG_MEMORY_OWNED)
	{
		if(_parentMemory)
			_parentMemory->Free(_memory,_endMemory-_memory);
//		else
//			free(_memory);
	}
}

//=============================================================================

void*
DMalloc::Allocate(size_t size ASSERTIONS( COMMA const char* file COMMA int line))
{
	Validate();
	assert(size);

	DBSTREAM1( cmem << "NEW," << size << ','; )

	if ( size & 3 )
		DBSTREAM1(cwarn << "DMalloc of " << size << " not long word aligned" << std::endl; )
	size += (4-(size&0x3))&3;
//	assert(ValidPtr(_memory+size));			// insure the size is ok for this architecture

	DBSTREAM5( cprogress << "DMalloc::Allocate called with size of " << size << std::endl; )

	DBSTREAM1( cmem << size << ','; )

	// loop through all free chunks looking for one large enough for this allocation
	FreeChunk* chunk = _firstChunk;
	FreeChunk* previousChunk = NULL;
#pragma message ("KTS " __FILE__ ": consider doing this in 2 loops, checking for exact fit first")
	while(chunk)
	{
		if((size+sizeof(AllocatedChunk)+(sizeof(FreeChunk))) <= chunk->Size() )
		{			                    // ok, allocate this chunk
			//printf("allocated memory at %p, size = %d\n",chunk,size+sizeof(AllocatedChunk));
			FreeChunk chunkCopy = *chunk;
			AllocatedChunk* allocatedChunk = new ((void*)chunk) AllocatedChunk(size);
			void* retVal = (void*)(allocatedChunk+1);
			FreeChunk* newFreeChunk = new (((char*)retVal)+size) FreeChunk(chunkCopy.Next(),chunkCopy.Size() - (size+sizeof(AllocatedChunk)));
			if(previousChunk)
				previousChunk->Next(newFreeChunk);
			else
				_firstChunk = newFreeChunk;
			ASSERTIONS( DBSTREAM1( cmem << file << ',' << line << "," << retVal << "," << Name() << std::endl; ) )
			return retVal;
		}

		if((size+sizeof(AllocatedChunk) == chunk->Size() ))
		{			                    // ok, allocate this chunk, but don't cut it into allocated+freed
			//printf("allocated memory at %p, size = %d\n",chunk,size+sizeof(AllocatedChunk));
			FreeChunk chunkCopy = *chunk;
			AllocatedChunk* allocatedChunk = new ((void*)chunk) AllocatedChunk(size);
			void* retVal = (void*)(allocatedChunk+1);
			if(previousChunk)
				previousChunk->Next(chunkCopy.Next());
			else
				_firstChunk = chunkCopy.Next();
			ASSERTIONS( DBSTREAM1( cmem << file << ',' << line << "," << retVal << "," << Name() << std::endl; ) )
			return retVal;
		}

		previousChunk = chunk;
		chunk = chunk->Next();
	}
	DBSTREAM1( cerror << *this << std::endl; )
	printf("DMalloc out of memory, request size = %d\n", size);
	MEMORY_NAMED(printf("Named %s",_name);)

	assert(0);
	FatalError("Dmalloc out of memory");
	return(NULL);                      					// hope somebody notices
}

//=============================================================================

inline void
DMalloc::Free(const void* mem)
{
	Validate();
	assert(ValidPtr(mem));

	RangeCheck(_memory,mem,_endMemory);
//	assert(mem >= _memory);
//	assert(mem < _endMemory);
	AllocatedChunk* allocatedChunk = ((AllocatedChunk*)mem)-1;
	assert((char*)allocatedChunk >= _memory);
	allocatedChunk->Validate();
	assert((char*)mem+allocatedChunk->Size() <= _endMemory);

	//printf("freeing memory at %p, size = %d\n",mem,allocatedChunk->Size());

	DBSTREAM5( cprogress << "DMalloc::Free called with ptr " << mem << std::endl; )

	DBSTREAM1( cmem << "DEL," << mem << ',' << Name() << std::endl; )

	// loop through free chunks, finding the two which stradle this memory
	FreeChunk* chunk = _firstChunk;
	FreeChunk* previousChunk = NULL;

	while(chunk)				    // find where in the list to put this chunk
	{
		if((char*)allocatedChunk < (char*)chunk)
			break;
		previousChunk = chunk;
		chunk = chunk->Next();
	}
	// ok, now we have pointers to before and after (either of which can be NULL)

	assert(previousChunk == NULL || (char*)previousChunk+previousChunk->Size() <= (char*)allocatedChunk);
	AssertMsg(chunk == NULL || (((char*)allocatedChunk)+sizeof(AllocatedChunk)+allocatedChunk->Size()) <= (char*)chunk,
		"allocatedChunk = " << allocatedChunk << ", chunk = " << chunk << ": Possibly freed a chunk which was already freed");

	bool makeNewChunk = true;
	// first check if adjacent to next chunk
	if(((char*)mem)+allocatedChunk->Size() == (char*)chunk)
	{
		// next yes
		makeNewChunk = false;
	}

	if(previousChunk && (char*)allocatedChunk == ((char*)previousChunk)+previousChunk->Size())
	{
		// coalesce with previous = yes
		makeNewChunk = false;
	}


#pragma message( __FILE__ ": this code always causes crash in release mode" )
	makeNewChunk = true;

	if(makeNewChunk)
	{
		// not next to any other free chunk, must add new one
		FreeChunk* newFreeChunk = new (allocatedChunk) FreeChunk(chunk, allocatedChunk->Size() + sizeof(AllocatedChunk));

		if(previousChunk)
			previousChunk->Next(newFreeChunk);
		else
			_firstChunk = newFreeChunk;
	}
	else
	{
		// must coalesce
		if(((char*)mem)+allocatedChunk->Size() == (char*)chunk)  // merge with previous
		{
			// now check if also adjacent to previous chunk
			if(previousChunk && (char*)allocatedChunk == ((char*)previousChunk)+previousChunk->Size())
			{
				// coalesce with previous and next chunk
				previousChunk->Next(chunk->Next());  // delete chunk from free list
				previousChunk->Size(previousChunk->Size() + allocatedChunk->Size() + sizeof(AllocatedChunk) + chunk->Size());
			}
			else
			{
				// just coalesce with next chunk
				FreeChunk* newChunk = new (allocatedChunk) FreeChunk(chunk->Next(), allocatedChunk->Size() + chunk->Size() + sizeof(FreeChunk));
				if(previousChunk)
					previousChunk->Next(newChunk);
				else
					_firstChunk = newChunk;
			}
		}
		// now check if adjacent to previous chunk
		else if(previousChunk && (char*)allocatedChunk == ((char*)previousChunk)+previousChunk->Size())
		{
			previousChunk->Size( previousChunk->Size() + allocatedChunk->Size() + sizeof(AllocatedChunk));
		}
#if DO_ASSERTIONS
		else
		{
		printf("DMalloc.cc: fucked\n");
			assert(0);
		}
#endif
	}
}

#if 0


	while(chunk)
	{
		if((void*)allocatedChunk < (void*)chunk)
		{			                    // ok, insert between these two chunks
#if DO_ASSERTIONS
			if(previousChunk)
			{
				assert((void*)previousChunk < (void*)allocatedChunk);
				assert((char*)previousChunk+previousChunk->Size() <= (char*)allocatedChunk);
			}
			AssertMsg(((char*)allocatedChunk+sizeof(AllocatedChunk)+allocatedChunk->Size()) <= (char*)chunk, "Possibly freed a chunk which was already freed");
#endif

#pragma message( __FILE__ ": this code causes crash in release mode" )
#if 0
			// first check if adjacent to next chunk
			if((char*)mem+allocatedChunk->Size() == (char*)chunk)
			{
				// now check if also adjacent to previous chunk
				if(previousChunk && (char*)allocatedChunk == (((char*)previousChunk)+previousChunk->Size()))
				{
					// coalesce with previous and next chunk
					previousChunk->Next(chunk->Next());  // delete chunk from free list
					previousChunk->Size(previousChunk->Size() + allocatedChunk->Size() + sizeof(AllocatedChunk) + chunk->Size());
					return;
				}

					// coalesce with next chunk
				FreeChunk* newChunk = new (allocatedChunk) FreeChunk(chunk->Next(),allocatedChunk->Size() + chunk->Size() + sizeof(FreeChunk));
				if(previousChunk)
					previousChunk->Next(newChunk);
				else
					_firstChunk = newChunk;
				return;
			}

			// now check if adjacent to previous chunk
			if(previousChunk && (char*)allocatedChunk == (((char*)previousChunk)+previousChunk->Size()))
			{
				previousChunk->Size( previousChunk->Size() + allocatedChunk->Size() + sizeof(AllocatedChunk));
				return;
			}
#endif

			FreeChunk* newFreeChunk = new (allocatedChunk) FreeChunk(chunk,allocatedChunk->Size() + sizeof(AllocatedChunk));
			if(previousChunk)
				previousChunk->Next(newFreeChunk);
			else
				_firstChunk = newFreeChunk;
			return;
		}
#error kts add code to handle freeing a chunk past the last freed chunk (currently fails)
		previousChunk = chunk;
		chunk = chunk->Next();
	}
	// must be at end (not likely)
	printf("DMalloc.cc: fucked\n");
	assert(0);
}
#endif

//=============================================================================

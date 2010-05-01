//==============================================================================
// lmalloc.cc
// Copyright ( c ) 1997,1998,1999,2000,2001,2003 World Foundry Group.  
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
// Sequential memory allocation class, usefull for temporary buffers which
// last a frame or so.
//============================================================================

#define _LMALLOC_CC
#include <memory/lmalloc.hp>
#include <cpplib/stdstrm.hp>
#include <streams/dbstrm.hp>
#include <cpplib/libstrm.hp>
#include <hal/hal.h>

#define LMALLOC_TRACK_SIZE 0
#define LMALLOC_TRACK_LINE_AND_FILE 0

#if LMALLOC_TRACK_LINE_AND_FILE
#if !MEMORY_TRACK_FILE_LINE
#error LMALLOC_TRACK_LINE_AND_FILE wont work without MEMORY_TRACK_FILE_LINE set
#endif
#endif

//=============================================================================

#if DO_ASSERTIONS
#if LMALLOC_TRACK_SIZE

struct FileLine
{
	enum
	{
		ALLOCATED = 'ALOC',
		FREED = 'FREE'
	};
	long _state;
	int _size;			                // size of allocation
#if LMALLOC_TRACK_LINE_AND_FILE
	char* _file;						// file and line allocation occured on
	int _line;
#endif
};
#endif

//==============================================================================

void
LMalloc::_Validate() const
{
	Memory::Validate();

	assert(ValidPtr(_memory));
	assert(ValidPtr((_endMemory-4)));
	AssertMsg(ValidPtr(_currentFree),"currentFree = " << _currentFree);
	assert(_currentFree >= _memory);
	assert(_currentFree < (_endMemory));
	assert(_flags == 0 || _flags == FLAG_MEMORY_OWNED);
}

#endif // DO_ASSERTIONS

//=============================================================================

#if DO_IOSTREAMS
std::ostream&
LMalloc::_Print(std::ostream& out) const
{
	out << "LMalloc Dump: [named " << Name() << ']' << std::endl;
	out << "_flags = " << _flags << std::endl;
	out << "_memory = " << (void*)_memory << std::endl;
	out << "_endMemory = " << (void*)_endMemory << std::endl;
	out << "_currentFree = " << (void*)_currentFree << std::endl;
	out << "_parentMemory = " << (void*)_parentMemory << std::endl;
	int cbUsed = _currentFree - _memory;
	int cbFree = _endMemory - _currentFree;
	int cbTotal = _endMemory - _memory;
	//assert( cbUsed + cbFree == cbTotal );
	out << cbUsed << '/' << cbTotal << " free=" << cbFree << std::endl;
	return out;
}

#endif

//=============================================================================

LMalloc::LMalloc(LMalloc& lmalloc, size_t size MEMORY_NAMED ( COMMA const char* name ) )
	: Memory( MEMORY_NAMED( name ) )
	// allocates from another lmalloc memory pool
{
	RangeCheckExclusive(0,size,1000000);  // kts arbitrary
	lmalloc.Validate();
	_memory = (char*)lmalloc.Allocate(size ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(_memory));
	AssertMemoryAllocation(_memory);
	//DBSTREAM1( printf("LMalloc::LMalloc: allocated %d bytes from lmalloc at address %p\n",size,_memory); )
	//printf("New LMalloc ");
	//MEMORY_NAMED( printf("named %s ",name); )
	//printf(" allocated %d bytes from lmalloc ",size);
	//MEMORY_NAMED( printf(" named %s ",lmalloc._name); )
	//printf(" at address %p\n",_memory);
	_endMemory = _memory + size;
	_currentFree = _memory;
	_flags = FLAG_MEMORY_OWNED;
	_parentMemory = &lmalloc;
	assert(ValidPtr(_parentMemory));
	Validate();
}

//=============================================================================

#if 0
LMalloc::LMalloc(size_t size MEMORY_NAMED( COMMA const char* name ) )
{
	assert(size);
	_memory = (char*)malloc(size);
	assert(ValidPtr(_memory));
	AssertMemoryAllocation(_memory);
	_endMemory = _memory + size;
	_currentFree = _memory;
	_flags = FLAG_MEMORY_OWNED;
	_parentMemory = NULL;
	MEMORY_NAMED( ValidatePtr(name);
		_name = name;
	)
	Validate();
	//DBSTREAM1( printf("LMalloc::LMaloc constructed from heap at address %p with a size of %d\n",_memory,size); )
	//printf("LMalloc ");
	//MEMORY_NAMED( printf(" named %s ",name); )
	//printf("constructed from heap at addr %p, size = %d\n",_memory,size);
}
#endif

//=============================================================================

LMalloc::LMalloc(void* memory, size_t size MEMORY_NAMED( COMMA const char* name ) )
	: Memory( MEMORY_NAMED( name ) )
{
	assert(size);
	assert(size >= 4);
	AssertMsg(ValidPtr(memory),"memory = " << memory);
	_memory = (char*)memory;
	assert(ValidPtr(_memory));
	_endMemory = _memory + size;
	assert(ValidPtr(_endMemory-4));
	_currentFree = _memory;
	_flags = 0;
	_parentMemory = NULL;
	Validate();
	//DBSTREAM1( printf("LMalloc::LMaloc constructed from pointer at address %p with a size of %d\n",_memory,size); )
	//printf("Lmalloc ");
	//MEMORY_NAMED( printf(" named %s ",_name); )
	//printf(" constructed from ptr at addr %p, size = %d\n",_memory,size);
}

//=============================================================================

LMalloc::~LMalloc()
{
#pragma message( "spew `DEL' tracking messages for all allocations" )
	Validate();
	if(_flags & FLAG_MEMORY_OWNED)
	{
		if(_parentMemory)
			_parentMemory->Free(_memory);
//		else
//			free(_memory);
	}
}

//=============================================================================

void*
LMalloc::Allocate(size_t size ASSERTIONS( COMMA const char* file COMMA int line))
{
	Validate();
	assert(size);

	DBSTREAM1( cmem << "NEW," << size << ','; )

#if DO_ASSERTIONS
#if LMALLOC_TRACK_SIZE
	size += sizeof(FileLine);
#endif
#endif

	if(size&3)
	{
		DBSTREAM1(cwarn << "LMalloc of " << size << " not long word aligned" << std::endl; )
	}
	size += (4-(size&0x3))&3;
	assert(ValidPtr(_memory+size));			// insure the size is ok for this architecture

	if((_currentFree + size) >= (_endMemory))
	{
		char errorBuffer[400];

		sprintf(errorBuffer, "Lmalloc based at address %p out of memory, request size = %d. lmalloc remaining = %d\n",_memory, size,_endMemory-_currentFree);
		MEMORY_NAMED ( sprintf(errorBuffer, "Lmalloc based at address %p out of memory, request size = %d. lmalloc remaining = %d, Named %s\n",_memory, size,_endMemory-_currentFree,_name); )
		DBSTREAM1( cerror << *this << std::endl; )
		AssertMsg(0,errorBuffer);
		FatalError(errorBuffer);
		//FatalError("Lmalloc out of memory");
		return(0);                      					// hope somebody notices
	}
	assert((_currentFree + size) < (_endMemory));		// if this fires, we are out of memory

	void* retVal = _currentFree;
	_currentFree += size;
#if DO_ASSERTIONS
#if LMALLOC_TRACK_SIZE
	FileLine* fl = (FileLine*)retVal;
	fl->_state = FileLine::ALLOCATED;
#if LMALLOC_TRACK_LINE_AND_FILE
	fl->_file = file;
	fl->_line = line;
#endif		// LMALLOC_TRACK_LINE_AND_FILE
	fl->_size = size;
	retVal = ((char*)retVal) + sizeof(FileLine);
#endif		// LMALLOC_TRACK_SIZE
#endif		// DO_ASSERTIONS

	ASSERTIONS( DBSTREAM1( cmem << size << ',' << file << ',' << line << "," << retVal << "," << Name() << std::endl; ) )

//	printf("memory allocated from lmalloc ");
//	DBSTREAM1( printf(" named %s ",_name); )
//	printf(" at %p,size = %d, left = %d\n",retVal, size, _endMemory-_currentFree);
	return(retVal);
}

//============================================================================

#define DUMPDATA

void
LMalloc::Free(const void* mem)
{
	Validate();

//	NEW|DEL, size, rounded_size, file, line, index, address, comments
//	cmem << "DEL," << 0 << ',' << 0 << ',' << "filename" << ",0," << mem << ',' << Name() << std::endl;
	DBSTREAM1( cmem << "DEL," << mem << ',' << Name() << std::endl; )

#if DO_ASSERTIONS
#if LMALLOC_TRACK_SIZE
	mem = ((char*)mem) - sizeof(FileLine);
#endif
#endif
	assert(ValidPtr(mem));
	AssertMsg(mem >= _memory, "mem = " << (void*)mem << ", _memory = " << _memory << "(Probably freed from wrong Memory instance");
	assert(mem < (_endMemory));

#if DO_ASSERTIONS
#if LMALLOC_TRACK_SIZE
	FileLine* fl = (FileLine*)mem;
	assert(fl->_state == FileLine::ALLOCATED);

	FileLine* nextfl = (FileLine*)(((char*)mem)+fl->_size);
	if(!(nextfl->_state == FileLine::ALLOCATED))
		nextfl = NULL;

	if(nextfl)
	{
		cerror << "LMalloc allocation mismatch:" << std::endl;
#if LMALLOC_TRACK_LINE_AND_FILE
		cerror << "should have freed: file = " << fl->_file << ", line = " << fl->_line << std::endl;
		cerror << "but tried to free: file = " << nextfl->_file << ", line = " << nextfl->_line << std::endl;
#endif			// LMALLOC_TRACK_LINE_AND_FILE
	}
	assert((_currentFree - fl->_size) == mem);
#endif			// LMALLOC_TRACK_SIZE
#endif			// DO_ASSERTIONS
//	AssertMsg((_currentFree - size) == mem, "_currentFree = " << (void*)_currentFree << ", size = " << size << ", mem  = " << mem ASSERTIONS( << ", file = " << fl->_file << ", line = " << fl->_line));			// can only free last allocated

   RangeCheck(_memory,mem,_endMemory);
   //assert(mem <= _currentFree);
	_currentFree = (char*)mem;
#if DO_ASSERTIONS
#if LMALLOC_TRACK_SIZE
	fl->_state = FileLine::FREED;
#endif
#endif
}

//=============================================================================

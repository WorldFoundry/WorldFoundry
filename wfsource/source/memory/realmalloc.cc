//==============================================================================
// RealMalloc.cc
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

#define _RealMalloc_CC
#include <memory/realmalloc.hp>
#include <cpplib/stdstrm.hp>
#include <streams/dbstrm.hp>
#include <hal/hal.h>
DBSTREAM1( extern ostream_withassign( cmem ); )

#define REALMALLOC_TRACK_SIZE 0
#define REALMALLOC_TRACK_LINE_AND_FILE 0

//=============================================================================

#if DO_ASSERTIONS
#if REALMALLOC_TRACK_SIZE

struct FileLine
{
	enum
	{
		ALLOCATED = 'ALOC',
		FREED = 'FREE'
	};
	long _state;
	int _size;			                // size of allocation
#if REALMALLOC_TRACK_LINE_AND_FILE
	char* _file;						// file and line allocation occured on
	int _line;
#endif
};
#endif

//==============================================================================

void
RealMalloc::_Validate() const
{
	Memory::Validate();

	assert(_allocationCount >= 0);
	assert(_flags == 0);
}

#endif // DO_ASSERTIONS

//=============================================================================

#if DO_IOSTREAMS
std::ostream&
RealMalloc::_Print(std::ostream& out) const
{
	out << "RealMalloc Dump: [named " << Name() << ']' << std::endl;
	out << "_flags = " << _flags << std::endl;
	out << "allocationCount = " << _allocationCount << std::endl;
	return out;
}

#endif

//=============================================================================

RealMalloc::RealMalloc( MEMORY_NAMED( const char* name ) )
	: Memory( MEMORY_NAMED( name ) )
{
	_allocationCount = 0;
	_flags = 0;
	Validate();
}

//=============================================================================

RealMalloc::~RealMalloc()
{
#pragma message( "spew `DEL' tracking messages for all allocations" )
	Validate();
}

//=============================================================================

void*
RealMalloc::Allocate(size_t size ASSERTIONS( COMMA const char* file COMMA int line))
{
	Validate();
	assert(size);

	DBSTREAM1( cmem << "NEW," << size << ','; )

#if DO_ASSERTIONS
#if RealMalloc_TRACK_SIZE
	size += sizeof(FileLine);
#endif
#endif

	if(size&3)
	{
		DBSTREAM1(cwarn << "RealMalloc of " << size << " not long word aligned" << std::endl; )
	}
	size += (4-(size&0x3))&3;


	void* retVal = malloc(size);
	if(!retVal)
	{
		char errorBuffer[400];

		sprintf(errorBuffer, "RealMalloc out of memory, request size = %d.\n", size);
		MEMORY_NAMED ( sprintf(errorBuffer, "RealMalloc out of memory, request size = %d. Named %s\n",size,_name); )
		DBSTREAM1( cerror << *this << std::endl; )
		AssertMsg(0,errorBuffer);
		FatalError(errorBuffer);
		//FatalError("RealMalloc out of memory");
		return(0);                      					// hope somebody notices
	}

#if DO_ASSERTIONS
#if RealMalloc_TRACK_SIZE
	FileLine* fl = (FileLine*)retVal;
	fl->_state = FileLine::ALLOCATED;
#if RealMalloc_TRACK_LINE_AND_FILE
	fl->_file = file;
	fl->_line = line;
#endif		// RealMalloc_TRACK_LINE_AND_FILE
	fl->_size = size;
	retVal = ((char*)retVal) + sizeof(FileLine);
#endif		// RealMalloc_TRACK_SIZE
#endif		// DO_ASSERTIONS

	ASSERTIONS( DBSTREAM1( cmem << size << ',' << file << ',' << line << "," << retVal << "," << Name() << std::endl; ) )

//	printf("memory allocated from RealMalloc ");
//	DBSTREAM1( printf(" named %s ",_name); )
//	printf(" at %p,size = %d, left = %d\n",retVal, size, _endMemory-_currentFree);
	_allocationCount++;
	return(retVal);
}

//============================================================================

#define DUMPDATA

void
RealMalloc::Free(const void* mem)
{
	Validate();

//	NEW|DEL, size, rounded_size, file, line, index, address, comments
//	cmem << "DEL," << 0 << ',' << 0 << ',' << "filename" << ",0," << mem << ',' << Name() << std::endl;
	DBSTREAM1( cmem << "DEL," << mem << ',' << Name() << std::endl; )

#if DO_ASSERTIONS
#if RealMalloc_TRACK_SIZE
	mem = ((char*)mem) - sizeof(FileLine);
#endif
#endif
	assert(ValidPtr(mem));

#if DO_ASSERTIONS
#if RealMalloc_TRACK_SIZE
	FileLine* fl = (FileLine*)mem;
	assert(fl->_state == FileLine::ALLOCATED);

	FileLine* nextfl = (FileLine*)(((char*)mem)+fl->_size);
	if(!(nextfl->_state == FileLine::ALLOCATED))
		nextfl = NULL;

	if(nextfl)
	{
		cerror << "RealMalloc allocation mismatch:" << std::endl;
#if RealMalloc_TRACK_LINE_AND_FILE
		cerror << "current free: file = " << fl->_file << ", line = " << fl->_line << std::endl;
		cerror << "stacked allocation: file = " << nextfl->_file << ", line = " << nextfl->_line << std::endl;
#endif			// RealMalloc_TRACK_LINE_AND_FILE
	}
	assert((_currentFree - fl->_size) == mem);
#endif			// RealMalloc_TRACK_SIZE
#endif			// DO_ASSERTIONS
//	AssertMsg((_currentFree - size) == mem, "_currentFree = " << (void*)_currentFree << ", size = " << size << ", mem  = " << mem ASSERTIONS( << ", file = " << fl->_file << ", line = " << fl->_line));			// can only free last allocated

#if DO_ASSERTIONS
#if REAL_TRACK_SIZE
	fl->_state = FileLine::FREED;
#endif
#endif
	free((void*)mem);
	_allocationCount--;
}

//=============================================================================

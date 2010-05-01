//==============================================================================
// Memory.cc
// Copyright ( c ) 1998,99 World Foundry Group.  
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
// Abstract Memory class, all memory allocators derive from this
//============================================================================

#define _Memory_CC
#include <memory/memory.hp>
#include <cpplib/stdstrm.hp>
#include <streams/dbstrm.hp>

//=============================================================================

Memory::Memory( MEMORY_NAMED( const char* name ) )
{
	MEMORY_NAMED(
		assert( ValidPtr( name ) );
		Name( name );
	)
//	printf("New Memory ");
}

//=============================================================================

Memory::~Memory()
{
//	Validate();
//	if(_flags & FLAG_MEMORY_OWNED)
//	{
//		if(_parentMemory)
//			_parentMemory->Free(_memory,_endMemory-_memory);
//		else
//			free(_memory);
//	}
}

//=============================================================================

#if DO_ASSERTIONS

void
Memory::_Validate() const
{
	assert( ValidPtr( _name ) );
}

#endif

//=============================================================================

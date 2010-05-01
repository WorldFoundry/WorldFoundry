//==============================================================================
// commonblock.cc:
// Copyright ( c ) 2003 World Foundry Group.  
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
// Description: all game objects need to be derived from this object
// since it is what the object list code refers to, as well as the asset handling code
//==============================================================================

#include <pigsys/pigsys.hp>
#include "commonblock.hp"

//==============================================================================

CommonBlock::CommonBlock(const void* commonBlockBase, int commonBlockSize) :
   _commonBlockBase(static_cast<const char*>(commonBlockBase)),
   _commonBlockSize(commonBlockSize)
{
   Validate();
}

//==============================================================================

const void*
CommonBlock::GetBlockPtr(int32 offset) const
{
   Validate();
   assert(_commonBlockSize);           // not allowed to look up if this is a NULL instance
   RangeCheck(0,offset,_commonBlockSize);
	assert( (offset & 3) == 0 );
	return( (void*)(_commonBlockBase + offset));
}

//==============================================================================


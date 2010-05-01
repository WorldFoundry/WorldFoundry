//============================================================================
// actroit.hpi:
//============================================================================
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

// ===========================================================================
// Original Author: Kevin T. Seghetti
//============================================================================

#include "actroit.hp"
#include <pigsys/assert.hp>

//============================================================================

ActiveRoomsBaseObjectIter::ActiveRoomsBaseObjectIter(const ActiveRooms& activeRooms, int listIndex)
:
	_activeRooms(activeRooms),
	currentActiveRoomIndex(0),
	_listIndex(listIndex),
	poIter(_activeRooms.GetActiveRoom(0)->ListIter(listIndex))
{
	Validate();
}

//==============================================================================

ActiveRoomsBaseObjectIter::~ActiveRoomsBaseObjectIter()
{

}

//==============================================================================

void
ActiveRoomsBaseObjectIter::_Validate() const
{
	_activeRooms.Validate();
   //RangeCheck(0,_listIndex,?);
	poIter.Validate();
}

//==============================================================================

BaseObject&
ActiveRoomsBaseObjectIter::operator*()					// dereference operator, returns the current PhysicalObject
{
	Validate();
   assert(!poIter.Empty());
	return *poIter;
}

//==============================================================================

BaseObjectIterator* 
ActiveRoomsBaseObjectIter::Copy() const
{
   ActiveRoomsBaseObjectIter* arpoi = new ActiveRoomsBaseObjectIter(_activeRooms,_listIndex);
   assert(ValidPtr(arpoi));
   return arpoi;
}

//==============================================================================


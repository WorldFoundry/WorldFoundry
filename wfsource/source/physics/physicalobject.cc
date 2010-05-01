//==============================================================================
// physicaLobject.cc:
// Copyright (c) 2002 World Foundry Group  
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
// Description: The PhysicalObject is a virtual base class, a client needs to derive
// from this and fill out the virutual functions to be able to use the movement handlers
// and collision system
// This is how I am able to decouple the physics from all of the other pieces it
// needs to access
// Original Author: Kevin T. Seghetti
//==============================================================================

#define _PHYSICALOBJECT_CC

#include <physics/physicalobject.hp>

//==============================================================================

#if SW_DBSTREAM >= 1

std::ostream&
operator << ( std::ostream& s, const PhysicalObject& obj )
{
	s << "PhysicalObject:" << std::endl;
   obj.Print(s);
	return s;
}

#endif // SW_DBSTREAM >= 1

//==============================================================================

void 
PhysicalObject::Collision(PhysicalObject& other, const Vector3& normal)
{
   // do nothing by default
}

//==============================================================================


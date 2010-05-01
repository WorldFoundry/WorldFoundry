//============================================================================
// Target.cc:
// Copyright ( c ) 1995,1996,1997,1999,2000,2002,2003 World Foundry Group  
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

//=============================================================================
//	Abstract:
//	History:
//			Created	From object.ccs using Prep
//
//	Class Hierarchy:
//			none
//
//	Dependancies:
//
//	Restrictions:
//
//	Example:
//
//============================================================================

#include <oas/target.ht>		// get oad structure information
#include "target.hp"
#include "actor.hp"

//============================================================================

Target::Target(const SObjectStartupData* startupData)
	: Actor(startupData)
{
}

//============================================================================

Actor::EActorKind
Target::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Target_KIND);
	return Actor::Target_KIND;
}

//============================================================================

void
Target::update()
{
}

//============================================================================

bool
Target::CanRender() const
{
	return false;
}

bool
Target::CanUpdate() const
{
	return false;
}


//============================================================================

Actor*
OadTarget(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Target(startupData);
}

//============================================================================

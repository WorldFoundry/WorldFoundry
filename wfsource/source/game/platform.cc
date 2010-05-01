//============================================================================
// Platform.cc:
// Copyright ( c ) 1995,1996,1997,1999,2000 World Foundry Group  
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

#include "platform.hp"
#include "actor.hp"
#include "oas/platform.ht"		// get oad structure information

//============================================================================

Platform::Platform(const SObjectStartupData* startupData)
	: Actor(startupData)
{
//	_isBreakaway = bool( getOad()->Breakaway );
}

//============================================================================

Actor::EActorKind
Platform::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Platform_KIND);
	return Actor::Platform_KIND;
}

void
Platform::initPath( QInputDigital*  )
{
//	Actor::initPath(level);
}

//============================================================================

Actor*
OadPlatform(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Platform(startupData);
}

//============================================================================


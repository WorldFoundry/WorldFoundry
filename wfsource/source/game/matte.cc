//============================================================================
// matte.cc:
// Copyright (c) 1995,1996,1997,1998,1999,2000,2002,2003 World Foundry Group  
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

#define _MATTE_CC

#include "level.hp"
#include "matte.hp"
#include <asset/assslot.hp>
#include <oas/matte.ht>

//============================================================================

//_Matte _bgMatte;

//============================================================================

Matte::Matte(const SObjectStartupData* startupData) : Actor(startupData), _matte(NULL)
{
}

//============================================================================

Matte::~Matte()
{
	if(_matte)
		MEMORY_DELETE(HALLmalloc,_matte,ScrollingMatte);
}

//============================================================================

Actor::EActorKind
Matte::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Matte_KIND);
	return Actor::Matte_KIND;
}

//============================================================================

Actor*
OadMatte( const SObjectStartupData* startupData )
{
//	_bgMatte = *(_Matte*)( startupData->objectData + 1 );
//	return NULL;
	return new (*startupData->memory) Matte(startupData);
}

//============================================================================

//============================================================================
// Destroy.cc:
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

#define _DESTROY_CC
#include <oas/destroyer.ht>
#include <oas/activate.ht>
#include "destroyer.hp"
#include "actor.hp"
#include <physics/activate.hp>
#include "gamestrm.hp"
#include "level.hp"

//==============================================================================

inline const _Activation*
Destroy::GetActivateBlockPtr() const
{
   assert(ValidPtr(getOad()));
   assert(ValidPtr(GetBlockPtr(getOad()->activatePageOffset)));
   return (_Activation*)GetBlockPtr(getOad()->activatePageOffset);
}
    
//============================================================================

Destroy::Destroy( const SObjectStartupData* startupData ) : 
  Actor( startupData ),
   activation(*GetActivateBlockPtr(), GetCommonBlock())
{

}

//============================================================================

BaseObject::EActorKind
Destroy::kind() const
{
	return BaseObject::Destroyer_KIND;
}

//============================================================================

void
Destroy::update()
{
	// kts added 7/15/97 15:15
	if ( !GetMailboxes().ReadMailbox( getOad()->ActivationMailBox ).AsBool() )
		return;

	Actor::update();

	const BaseObject* colObject = activation.Activated(GetPhysicalAttributes(), theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE), *GetActivateBlockPtr(), theLevel->GetObjectList() );
	if ( colObject )
	{
		DBSTREAM3( cstats << *this << " destroying " << *colObject << std::endl; )
		theLevel->SetPendingRemove( colObject);
	}
}

//============================================================================

Actor*
OadDestroyer(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Destroy(startupData);
}

//============================================================================

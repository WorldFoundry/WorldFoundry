//============================================================================
// ActBoxOR.cc: Activation Box Object References
// Copyright ( c ) 1996,1997,1999,2000,2002,2003 World Foundry Group  
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
//
//	Abstract:
//	History:
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

#define _ACTBOXOR_CC
#include <oas/actboxor.ht>		// get oad structure information
#include <oas/activate.ht>
#include "actboxor.hp"
#include "actor.hp"
#include <physics/activate.hp>

//============================================================================

inline const _Activation*
ActBoxOR::GetActivateBlockPtr() const
{
   assert(ValidPtr(getOad()));
   assert(ValidPtr(GetBlockPtr(getOad()->activatePageOffset)));
   return (_Activation*)GetBlockPtr(getOad()->activatePageOffset);
}
    
//==============================================================================
   
ActBoxOR::ActBoxOR(const SObjectStartupData* startupData) : 
  Actor(startupData),
  activation(*GetActivateBlockPtr(), GetCommonBlock())
{
}

//============================================================================

Actor::EActorKind
ActBoxOR::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::ActBoxOR_KIND);
	return Actor::ActBoxOR_KIND;
}

//============================================================================

void
ActBoxOR::update()
{
	DBSTREAM1( cnull << "actboxor::update on " << *this << std::endl; )

	const BaseObject* colObject = activation.Activated( GetPhysicalAttributes(), theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE), *GetActivateBlockPtr(), theLevel->GetObjectList());
	if ( colObject )
	{
		DBSTREAM1( cnull << "actboxor update: writing " << getOad()->Object << " to mailbox" << getOad()->MailBox << std::endl; )

		assert(getOad()->Object);
		assert(getOad()->Object > 0);
		assert(getOad()->Object < theLevel->GetMaxObjectIndex() );

        GetMailboxes().WriteMailbox( getOad()->MailBox, Scalar( getOad()->Object, 0 ) );
		//std::cout << "Setting mailbox " << getOad()->MailBox << " to " << Scalar( getOad()->Object, 0 ) << std::endl;
	}
	Actor::update();
	// FIX -- hack until order execution priority is enabled because actboxes
	// are executed before all other objects (and prepareToRender isn't called at all)
	GetWritablePhysicalAttributes().HasRunUpdate(false);
}

//============================================================================

Actor*
OadActBoxOR(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) ActBoxOR(startupData);
}

//============================================================================

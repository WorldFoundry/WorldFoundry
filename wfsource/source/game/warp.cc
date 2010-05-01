//============================================================================
// Warp.cc:
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

#define _WARP_CC
#include <oas/warp.ht>		// get oad structure information
#include <oas/activate.ht>
#include "warp.hp"
#include "actor.hp"
#include "level.hp"
#include <physics/activate.hp>

//============================================================================

inline const _Activation*
Warp::GetActivateBlockPtr() const
{
   assert(ValidPtr(getOad()));
   assert(ValidPtr(GetBlockPtr(getOad()->activatePageOffset)));
   return (_Activation*)GetBlockPtr(getOad()->activatePageOffset);
}
    
//==============================================================================
                          
Warp::Warp(const SObjectStartupData* startupData)
	: Actor(startupData),
   activation(*GetActivateBlockPtr(), GetCommonBlock())

{
}

//============================================================================

Actor::EActorKind
Warp::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Warp_KIND);
	return Actor::Warp_KIND;
}

//============================================================================

void
Warp::update()
{
	BaseObject* colObject = activation.Activated( GetPhysicalAttributes(), theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE), *GetActivateBlockPtr(), theLevel->GetObjectList() );
	if ( colObject )
	{
		DBSTREAM5( cdebug << "Warp::update: activated by object " << colObject << std::endl; )
		const BaseObject* bo = theLevel->GetObject( getOad()->Target );
		assert( ValidPtr( bo ) );
      const PhysicalObject* target = dynamic_cast<const PhysicalObject*>(bo);
		assert( ValidPtr( target ) );

      PhysicalObject* po = dynamic_cast<PhysicalObject*>(colObject);
      assert(ValidPtr(po)); 

		DBSTREAM1( if ( !po->GetMovementBlockPtr()->Mobility )
			cerror << *this << " tried to move anchored " << *colObject << std::endl; )
		po->GetWritablePhysicalAttributes().SetPredictedPosition( target->GetPhysicalAttributes().Position() );
	}
	Actor::update();

#if 0
//	Actor::update( level );

	DBSTREAM5( cdebug << "Warp::update()" << std::endl; )

	int16 msgType;
	int32 msgData;
	while ( _nonStatPlat->_msgPort.GetMsg( msgType, msgData ) )
	{
		DBSTREAM3( cdebug << "Warp::update: message" << std::endl; )
	 	switch(msgType)
		{
			case MsgPort::COLLISION:
				assert(0);
				break;
			case MsgPort::SPECIAL_COLLISION:
			{
				Actor* colActor = (Actor*)msgData;
				DBSTREAM4( cdebug << "Warp::update: collision with actor  " << colActor << std::endl; )
				if ( activation.Activated(theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE), (struct _Activation*)&GetActivateBlockPtr()->ActivatedBy, colActor, *GetActivationBlockPtr(), theLevel->GetObjectList()) )
				{
					DBSTREAM5( cdebug << "Warp::update: activated by " << colActor << std::endl; )
					BaseObject* bo = theLevel->GetObject( getOad()->Target );
               const PhysicalObject* target = dynamic_cast<PhysicalObject*>(bo);
               assert( ValidPtr( target ) );
					assert( target );
					DBSTREAM1
					(	if ( !target->GetMovementBlockPtr()->Mobility )
							cerror << *this << " tried to move anchored " << *target << std::endl;
					)
					colActor->GetWritablePhysicalAttributes().
						SetPredictedPosition( target->currentPos() );
				}
				break;
			}

			default:
				DBSTREAM1( cdebug << "msgType = " << msgType << std::endl; )
				AssertMsg( 0, *this << ": Warp::update() : Unknown message type" << std::endl );
				break;
		}
	}
#endif
}

//============================================================================

Actor*
OadWarp(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Warp(startupData);
}

//============================================================================

//============================================================================
// Spike.cc:
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

#include <oas/spike.ht>
#include <oas/activate.ht>
#include "spike.hp"
#include "actor.hp"				// get object enumeration
#include "gamestrm.hp"
#include <physics/activate.hp>

//============================================================================

inline const _Activation*
Spike::GetActivateBlockPtr() const
{
   assert(ValidPtr(getOad()));
   assert(ValidPtr(GetBlockPtr(getOad()->activatePageOffset)));
   return (_Activation*)GetBlockPtr(getOad()->activatePageOffset);
}
    
//============================================================================

Spike::Spike(const SObjectStartupData* startupData)
	: Actor(startupData),
   activation(*GetActivateBlockPtr(), GetCommonBlock())
{
}

//============================================================================

void Spike::update()
{
	DBSTREAM1( cactor << "Entering Spike::update()." << std::endl; )

//	if (hasRunUpdate)		// bail if this actor has already been updated
//		return;

//	hasRunUpdate = true;	// set flag so we won't be updated again this frame

   char msgData[msgDataSize];

//	while ( _msgPort.GetMsgByType( MsgPort::SPECIAL_COLLISION, msgData ) )
	while ( GetSpecialCollisionMessage(&msgData,msgDataSize) )
	{
      PhysicalObject* colObject = (PhysicalObject*)msgData;
		if ( colObject )
		{
			assert( ValidPtr( colObject ) );

			DBSTREAM4( cdebug << "Spike::update: collision with " << colObject << std::endl; )
			if ( activation.IsActivated(*GetActivateBlockPtr(),colObject, theLevel->GetObjectList()))
			{
				if ( getOad()->HealthModifier != 0 )
				{
					//DBSTREAM1( cerror << "spike doing damage of " << getOad()->HealthModifier << std::endl; )
					colObject->sendMsg( MsgPort::DELTA_HEALTH, getOad()->HealthModifier );
				}

#pragma message( "BUBBA: kludge" )
				theLevel->SetPendingRemove(this);						// our work here is done
			}
			else
				_nonStatPlat->_msgPort.PutMsg( MsgPort::COLLISION, msgData,msgDataSize );		// otherwise, convert this into a regular collision message
		}
	}
	Actor::update();
}

//============================================================================
Actor::EActorKind
Spike::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Spike_KIND);
	return Actor::Spike_KIND;
}

//============================================================================

Actor*
OadSpike(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Spike(startupData);
}

//============================================================================

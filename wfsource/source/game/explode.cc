//============================================================================
// explode.cc:
// Copyright (c) 1994,1995,1996,1997,1999,2000,2002,2003 World Foundry Group  
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

#define _EXPLODE_CC

#include "level.hp"
#include "actor.hp"
#include "explode.hp"
#include "oas/explode.ht"

//==============================================================================

Explode::Explode(const SObjectStartupData* startupData) : Actor(startupData)
{
	AssertMsg( ((GetMovementBlockPtr()->Mass == Scalar::zero) && (getOad()->HealthModifier == 0)) ||
			   ((GetMovementBlockPtr()->Mass != Scalar::zero) && (getOad()->HealthModifier != 0)),
		*this << ": Mass (" << GetMovementBlockPtr()->Mass << ") and Health Modifier (" <<
		getOad()->HealthModifier << ") should either both be zero or both be non-zero" );
	_owner = 0;
	_timeDone = startupData->currentTime.Current() + _nonStatPlat->_animManager->GetCycleDuration();
}

//==============================================================================

Actor::EActorKind
Explode::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Explode_KIND);
	return Actor::Explode_KIND;
}

//==============================================================================

void
Explode::update()
{
   char msgData[msgDataSize];

	while ( _nonStatPlat->_msgPort.GetMsgByType( MsgPort::SPECIAL_COLLISION, &msgData, msgDataSize ) )
	{
		Actor* colActor = (Actor*)msgData;
		AssertMsg( ValidPtr( colActor ), *this << " got an invalid collision message with an invalid pointer: " << std::hex << colActor );

		if ( getOad()->HealthModifier )
		{
			if ( colActor->kind() == kind() )
			{	// Collision with explosion causes greater explosion
			}
			else
			{	// An actor type other than explosion
//				cerror << "explosion sending " << getOad()->HealthModifier;
//				cerror << " health modifier to " << colActor << std::endl;
				colActor->sendMsg( MsgPort::DELTA_HEALTH, getOad()->HealthModifier );

				// What's an explosion without bodies flying through the air?
				if (colActor->GetMovementBlockPtr()->Mobility)
				{
					Vector3 posVector = colActor->currentPos() - this->currentPos();
					posVector *= (Scalar::one / posVector.Length());	// force as inverse of distance
					if (posVector.Z() == Scalar::zero)
						posVector.SetZ(Scalar::one);	// make sure the explosion has some "loft"

					posVector *= getOad()->GetForce();

#pragma message("This isn't really a BR_SCALAR.  Scalar needs to be cleaned up.")
					colActor->sendMsg( MsgPort::MOVEMENT_FORCE_X, &posVector.X(),sizeof(Scalar) );
					colActor->sendMsg( MsgPort::MOVEMENT_FORCE_Y, &posVector.Y(),sizeof(Scalar) );
               Scalar temp = posVector.Z().Abs();
					colActor->sendMsg( MsgPort::MOVEMENT_FORCE_Z, &temp,sizeof(Scalar) );
				}
			}
		}
	 }

#if defined(RENDERER_BRENDER)
	_vactor.PostScale( Vector3::one );
#endif
#if defined(RENDERER_BRENDER)
	assert(0);
#endif

	if ( theLevel->LevelClock().Current() > _timeDone )
	{	// done exploding,  remove from world
		_nonStatPlat->_hitPoints = Scalar::zero;							// Tell script its going away this frame
#pragma message( __FILE__ ": this is where a message would be sent to script" )
		theLevel->SetPendingRemove( this );
		DBSTREAM1( cactor << "removing explode: " << this << std::endl; )
	}

	Actor::update();
}

//============================================================================

Actor*
OadExplode( const SObjectStartupData* startupData )
{
	return new (*startupData->memory) Explode(startupData);
}

//============================================================================

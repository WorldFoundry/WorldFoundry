//============================================================================
// Copyright (c) 1997,1999,2000,2002,2003 World Foundry Group.  
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

//============================================================================
// Description: AnimationManager watches the state of the game and chooses
// animations accordingly (for each object)
//============================================================================

#define _ANIMATIONMANAGER_CC
#include <renderassets/rendacto.hp>

#include <input/inputdig.hp>
#include <physics/physical.hp>
#include <physics/physicalobject.hp>
#include <movement/inputmap.hp>
#include <movement/movement.hp>
#include "animmang.hp"

//===========================================================================

AnimationManager::~AnimationManager()
{
}

//===========================================================================

AnimationManagerNull AnimationManagerNull::theAnimationManagerNull;	// single global instance for all non-animating objects

//===========================================================================

AnimationManagerActual::AnimationManagerActual() :
	_movementState( MovementObject::Uninitialized_state ),
	_previousMovementState( MovementObject::Uninitialized_state ),
	_currentCycleEndTime( Scalar::zero ),
	_overrideEndTime( Scalar::zero ),
	_overrideAnimCenter( Vector3::zero ),
	_overrideCenterHandle( "cent" ),
	_overrideAnimCycle( AnimationManagerActual::IDLE ),
	_animCenter( Vector3::zero ),		//Scalar::zero, Scalar::zero, Scalar::zero ),
	_centerHandle( "cent" )
{

}

//===========================================================================

AnimationManagerActual::~AnimationManagerActual()
{
}

//===========================================================================

void
AnimationManagerActual::SetAnimCenter( const Vector3& animCenter, HandleID centerHandle )
{
	_animCenter = animCenter;
	_centerHandle = centerHandle;
}

//==============================================================================

void
AnimationManagerActual::SetMovementState( const MovementObject::EMovementState newState )
{
	_previousMovementState = _movementState;
	_movementState = newState;
}

//==============================================================================

void
AnimationManagerActual::SetAnimCycle( RenderActor& renderActor, int newAnimCycle, const Clock& currentTime )
{
	if (_overrideEndTime <= currentTime.Current())
	{
		renderActor.SetAnimationCycle(newAnimCycle);
		_currentCycleEndTime = currentTime.Current() + GetCycleDuration();
	}
}

//==============================================================================

Scalar
AnimationManagerActual::GetCycleDuration( )
{
	Scalar retVal = SCALAR_CONSTANT(0);
	return retVal;
}

//==============================================================================

void
AnimationManagerActual::OverrideAnimCycle( int newAnimCycle, const Clock& currentTime )
{
	_overrideAnimCycle = newAnimCycle;
	_overrideEndTime = GetCycleDuration() + currentTime.Current();
}

//==============================================================================

void
AnimationManagerActual::UpdateAnimation( RenderActor* renderActor, const QInputDigital& inputDevice, PhysicalObject& physicalObject, BaseObjectIteratorWrapper& objectIter, const MovementHandlerData* handlerData, const Clock& currentTime)
{
   const PhysicalAttributes& physicalAttributes = physicalObject.GetPhysicalAttributes();
	int newAnimationCycle = AnimationManagerActual::IDLE;
	joystickButtonsF buttons = inputDevice.arePressed();
	AssertMsg(ValidPtr(handlerData), physicalObject << " has invalid movement handler data");
	AssertMsg(ValidPtr(renderActor), physicalObject << " has invalid renderActor");
	Scalar wallClock = currentTime.Current();
//	bool needsDeltaTimeApplied = true;

	// Check to see if an animCycle override is in effect
	if (_overrideEndTime > wallClock)
	{
		switch (_overrideAnimCycle)		// do something special based on the anim cycle
		{
			case AnimationManagerActual::SWORD_STRIKE_FORWARD:
			{
#define SWORD_DAMAGE (-10)
				// We're still swinging the sword.  Check the sword tip against all Enemy actors.
				Vector3 swordLocation;

            bool validHandle;
            if ( validHandle = renderActor->GetHandle(HandleID("sord"), swordLocation) )
               swordLocation += physicalAttributes.Position();

		 		AssertMsg( validHandle, "This Animation doesn't have a 'sord' handle!" );

				//  Now check all Enemies in the list for collision with the handle

				PhysicalObject* colObject;
				while ( !objectIter.Empty() )
				{
					colObject = dynamic_cast<PhysicalObject*>(&(*objectIter));
					assert( ValidPtr( colObject ) );
					if ( colObject->kind() == BaseObject::Enemy_KIND )
					{
						const PhysicalAttributes& pa = colObject->GetPhysicalAttributes();

						if ( pa.CheckCollision( swordLocation ) )
						{
							colObject->sendMsg( MsgPort::DELTA_HEALTH, SWORD_DAMAGE );
						}
					}
					++objectIter;
				}
				break;
			}

			default:
				break;
		}
	}
	else
	{
		switch (_movementState)
		{
			case MovementObject::Ground_state:
			{
				SetAnimCenter( Vector3(Scalar::zero, Scalar::zero,
								physicalAttributes.GetColSpace().UnExpMin(physicalAttributes.Position()).Z()), "cent" );

				// Choose walk/run animation based on ground speed
//				assert( thisActor.anim );
				Scalar wheelSpeed = handlerData->wheelVelocity.Length();
				Scalar mediumThreshold = Scalar::FromFixed32(physicalObject.GetMovementBlockPtr()->MaxGroundSpeed) * SCALAR_CONSTANT(0.3);

				if ( wheelSpeed > SCALAR_CONSTANT(0.01) )
					newAnimationCycle = AnimationManagerActual::RUN_SOFT;

				if ( wheelSpeed > mediumThreshold )
					newAnimationCycle = AnimationManagerActual::RUN_MEDIUM;

				if ( wheelSpeed > (mediumThreshold * SCALAR_CONSTANT(2.0)) )
					newAnimationCycle = AnimationManagerActual::RUN_HARD;

				// Are we sliding to a stop?
				#define SLIDING_FUDGE SCALAR_CONSTANT(0.125)
				if ( (wheelSpeed + SLIDING_FUDGE) < handlerData->relativeVelocity.Length() )
			 		newAnimationCycle = AnimationManagerActual::RUN_MED_STOP_SHORT;

				// override if sidestepping
				if (buttons & kBtnStepLeft)
					newAnimationCycle = AnimationManagerActual::SIDESTEP_LEFT;
				else if (buttons & kBtnStepRight)
					newAnimationCycle = AnimationManagerActual::SIDESTEP_RIGHT;

				// Check for sword-thrust move.
				// This is only thrust forward for now (will thrust to four directions based on actbox collisions)
				if (inputDevice.justPressed() & kBtnSword)
					OverrideAnimCycle(AnimationManagerActual::SWORD_STRIKE_FORWARD,currentTime);

				break;
			}

			case MovementObject::Air_state:
			{
				SetAnimCenter( Vector3::zero, "cent" );

				Scalar fallAnimThreshold = Scalar::FromFixed32(physicalObject.GetMovementBlockPtr()->FallAnimThreshold);

				if (physicalAttributes.LinVelocity().Z() < (Scalar::zero - fallAnimThreshold))
					newAnimationCycle = AnimationManagerActual::FALL;

				if ( (physicalAttributes.LinVelocity().Z() < -(fallAnimThreshold * SCALAR_CONSTANT(5))) && (buttons & kBtnJump) )
				{
					newAnimationCycle = AnimationManagerActual::FALL_FAST;
				}

				// Did we get here by jumping?
				if (inputDevice.justPressed() & kBtnJump)
				{
					Vector3 groundVelocity(physicalAttributes.LinVelocity());
					groundVelocity.SetZ(Scalar::zero);
					if (groundVelocity.Length() > Scalar::two)
						newAnimationCycle = AnimationManagerActual::JUMP;
					else
						newAnimationCycle = AnimationManagerActual::JUMP_UP;

					OverrideAnimCycle(newAnimationCycle, currentTime);
				}

				break;
			}

			case MovementObject::Climb_state:
			{
				break;
			}

			case MovementObject::Uninitialized_state:
				DBSTREAM1( cerror << "_movementState uninitialized!" << std::endl; )
            assert(0);
				break;

			default:
				AssertMsg(0, "Unknown movement state");
				break;
		}

		SetAnimCycle( *renderActor, newAnimationCycle, currentTime );

		if (newAnimationCycle == AnimationManagerActual::IDLE)
			_currentCycleEndTime = Scalar::zero;
		else
			_currentCycleEndTime = wallClock + GetCycleDuration();
	}

#pragma message ("KTS: new anim system")
//	if (needsDeltaTimeApplied)
//		thisActor.anim->ApplyDeltaTime( currentTime.Delta() );

#if 0
// Taken from AirHandler::init()
	int32 newAnimCycle = AnimationManagerActual::IDLE;

	if ( airData->jumpDuration > Scalar::zero )
	{
		thisActor->_animHandlerData = thisActor->currentPos().Z();
	}
	else if (physicalAttributes->LinVelocity().Z() < SCALAR_CONSTANT(-2.0) )
		newAnimCycle = AnimationManagerActual::FALL;

	thisActor->anim->SetAnimationCycle( newAnimCycle );
	thisActor->_animManager.SetAnimCenter( Vector3::zero, "cent" );


// Taken from AirHandler::update()
				if (sortedCollisionArray[colIndex].collisionSpeed > stunThreshold)
				{
					thisActor->anim->SetAnimationCycle( AnimationManagerActual::LAND_SOFT );
					thisActor->anim->ApplyTime( Scalar::zero );
					((GroundHandler::GroundHandlerData*)airData)->stunnedUntil = theTime + thisActor->anim->Duration();
				}

				if (sortedCollisionArray[colIndex].collisionSpeed > (stunThreshold * Scalar::two))
				{
					thisActor->anim->SetAnimationCycle( AnimationManagerActual::LAND_HARD );
					thisActor->anim->ApplyTime( Scalar::zero );
					((GroundHandler::GroundHandlerData*)airData)->stunnedUntil = theTime + thisActor->anim->Duration();
				}





// Taken from ClimbHandler::init()
	thisActor->_animManager.SetAnimCenter( Vector3::zero, "cent" );
	thisActor->anim->SetAnimationCycle( AnimationManagerActual::CLIMB_PULL_UP );


// Taken from ClimbHandler::update()
	Scalar distance = ((physicalAttributes->MaxZ() - otherAttr->MaxZ()) * SCALAR_CONSTANT(10));

	if (distance <= Scalar::zero)		// go to hang animation if fully extended
	{
		thisActor->anim->SetAnimationCycle( AnimationManagerActual::HANG );
		thisActor->anim->ApplyDeltaTime( deltaT );
	}
	else	// continue climbing
	{
		int32 tableIndex;
		memcpy(&tableIndex, &distance, sizeof(int32));
		tableIndex = tableIndex >> 16;
		Scalar time = CLIMB_FRAME_TABLE[tableIndex];
		thisActor->anim->SetAnimationCycle( AnimationManagerActual::CLIMB_PULL_UP );
		if (time > thisActor->anim->Duration())
			time = thisActor->anim->Duration();
		thisActor->anim->ApplyTime(time);
	}
#endif

	// Our own anim cycle choice may be overridden by an explicit message
    char msgData[msgDataSize];
	if (physicalObject.GetMsgPort().GetMsgByType(MsgPort::SET_ANIM_CYCLE, &msgData,msgDataSize))
   {
		SetAnimCycle(*renderActor,  *(int32*)&msgData,currentTime);
   }

	// Just like SET_ANIM_CYCLE, but sets override timer
	if (physicalObject.GetMsgPort().GetMsgByType(MsgPort::OVERRIDE_ANIM_CYCLE, &msgData, msgDataSize))
   {
		OverrideAnimCycle(*(int32*)&msgData, currentTime);
   }
}


//===========================================================================
// AnimationManagerNull is a dummy class that doesn't do anything (except absorb
// calls to AnimationManager functions)

void
AnimationManagerNull::UpdateAnimation(RenderActor* , const QInputDigital& , PhysicalObject&,  BaseObjectIteratorWrapper& , const MovementHandlerData*, const Clock& )
{
}

void
AnimationManagerNull::SetAnimCenter( const Vector3&, HandleID  )
{
}

void
AnimationManagerNull::SetMovementState( const MovementObject::EMovementState )
{
}

void
AnimationManagerNull::SetAnimCycle( RenderActor&, int, const Clock& )
{
}

Scalar
AnimationManagerNull::GetCycleDuration()
{
	return SCALAR_CONSTANT(0);
}

void
AnimationManagerNull::OverrideAnimCycle( int, const Clock& )
{
}

//===========================================================================

//============================================================================
// toolngun.cc:
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
//			Needle Gun class for game
//	History:
//			Created	From Tool.cc
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

#include <oas/tool.ht>		// get oad structure information
#include "tool.hp"
#include "toolngun.hp"

//============================================================================

ToolNeedleGun::ToolNeedleGun(const SObjectStartupData* startupData)
	: Tool(startupData)
{
}

//============================================================================

void
ToolNeedleGun::activate()
{
	DBSTREAM1( ctool << "ToolNeedleGun::Activate" << std::endl; )

	if ( getOad()->Type == TOOL_TYPE_BEAM )
	{
		const _Tool * oad = getOad();
	//	brm: PSX compiler will not eat this:
	//		Angle shootAngle( Angle::Revolution( oad->BeamSpreadAngle ) / 2 );
	//  so I changed it to:
		Angle shootAngle( Angle::Revolution( oad->GetBeamSpreadAngle() / Scalar::two ) );
	#ifdef ACTUAL_ANGLE_FOR_SHOOTING
		Angle faceAngle( GetPhysicalAttributes().RotationC() );
	#else	// use the "ideal" direction, the "desired" angle
		Angle faceAngle( getIdealAngle() );
	#endif
		DBSTREAM1( ctool << "faceAngle == " << faceAngle << std::endl; )

		struct hitSet {
			Actor *actor;
			Scalar dist;
			Angle angle;
		};

		struct hitSet choice;
		struct hitSet candidate;

		assert(_owner);
		assert(ValidPtr(theLevel->getActor(_owner)));

		choice.actor = NULL;
		choice.dist = getOad()->GetMaxRange();
		choice.angle = shootAngle;

      BaseObjectIteratorWrapper objects = theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE);
		assert (!objects.Empty());

		while( !objects.Empty() )
		{
         Actor* actor = dynamic_cast<Actor*>(&(*objects));
         assert(ValidPtr(actor));
			if ( actor->IsNeedleGunTarget() )
			{
				candidate.actor = actor;
				DBSTREAM3( ctool << "considering actor # " << candidate.actor->GetActorIndex() << std::endl; )
				candidate.angle = getRelativeAngle(candidate.actor, faceAngle);
				if( candidate.angle > Angle::Revolution( SCALAR_CONSTANT( 0.5 ) ) )
				{
					DBSTREAM1( ctool << "candidate.angle == " << std::endl; )
					DBSTREAM1( ctool << candidate.angle << " / " << std::endl; )
					candidate.angle = -candidate.angle; // make angle 0-180
					DBSTREAM1( ctool << candidate.angle << std::endl; )
				}

				// test whether or not guard is in effective radius of weapon
				// zone determines width (reciprocal of 360/arc)
				// divisor == SCALAR_CONSTANT(1.0/8) means 1/8 of circle,
				// Scalar(0.25) means 1/4, etc.
				// radius is divided to 1/2 positive and 1/2 negative rotation.
				DBSTREAM3( ctool << "canidate angle = " << candidate.angle << ", shootAngle = " << shootAngle << std::endl; )

				if (candidate.angle < shootAngle) // test if within the zone
				{
					assert(_owner);
					assert(ValidPtr(theLevel->getActor(_owner)));
					candidate.dist = theLevel->getActor(_owner)->GetDistanceTo(candidate.actor);
					if (candidate.dist < choice.dist)
					{
					// if candidate is closer:
						// if angle is smaller then ALWAYS select.
						if (candidate.angle <= choice.angle)
						{
							choice = candidate;
						}
						// if angle is greater then select IF (candidate.dist < delta_dist).
						else
						{
							if (candidate.dist < (choice.dist - candidate.dist))
							{
								choice = candidate;
							}
						}
					}
					else
					{
					// if candidate is farther:
						// if angle is greater then NEVER select.

						// if angle is smaller then select IF (choice.dist > delta_dist).
						if (candidate.angle < choice.angle)
						{
							if (choice.dist > (candidate.dist - choice.dist))
							{
								choice = candidate;
							}
						}
					}
				}
			}
			++objects;
		}
		if (choice.actor != NULL)	// if a choice is made
		{	// put the object at target's center
			if (_objectToGenerate != -1)
			{
				Actor* createdObject = theLevel->ConstructTemplateObject(_objectToGenerate, _owner, choice.actor->currentPos());
//				const SObjectStartupData* startupData = theLevel->FindTemplateObjectData(_objectToGenerate);
//				Actor* createdObject = ConstructTemplateObject(startupData->objectData->type,startupData);
				assert(ValidPtr(createdObject));
				theLevel->AddObject( createdObject, choice.actor->currentPos() );
			}
			DBSTREAM1( ctool << "Ya got 'im" << std::endl; )
		}
	}

	// Run this tool's activation script (if it has one)
	if (_pScript)
      theLevel->EvalScript(_pScript,GetActorIndex());
}

//============================================================================
// returns angle to an actor relative to given direction
Angle ToolNeedleGun::getRelativeAngle(Actor *target, Angle baseAngle)
{
	Vector3 delta(target->currentPos() - currentPos());
//	Angle angle( delta.X().ATan2(-delta.Y()) - baseAngle );
	Angle angle( delta.Y().ATan2(delta.X()) - baseAngle );

	DBSTREAM1( ctool << "RelAngle == " << angle << std::endl; )
	DBSTREAM1( ctool << "BaseAngle == " << baseAngle << std::endl; )

	return (angle);
}

//============================================================================
// returns angle to an actor relative to current direction
Angle ToolNeedleGun::getRelativeAngle(Actor *target)
{

#ifdef ACTUAL_ANGLE_FOR_SHOOTING
	return getRelativeAngle(target, GetPhysicalAttributes().RotationC());
#else	// use the "ideal" direction, the "desired" angle
	return getRelativeAngle(target, getIdealAngle());
#endif

}


//============================================================================
// returns the angle that the joystick wants to face,
// or the actual angle if no buttons.

Angle ToolNeedleGun::getIdealAngle()
{
	assert(_owner);
	assert(ValidPtr(theLevel->getActor(_owner)));
	joystickButtonsF buttons = theLevel->getActor(_owner)->GetInputDevice()->arePressed();

	DBSTREAM5( ctool << "buttons == " << buttons << std::endl; )

	if ( (buttons & EJ_BUTTONF_UP) && (buttons & EJ_BUTTONF_RIGHT) )
		return Angle::Revolution( SCALAR_CONSTANT( 0.625 ) );

	if ( buttons & EJ_BUTTONF_RIGHT )
		return Angle::Revolution( SCALAR_CONSTANT( 0.5 ) );

	if ( (buttons & EJ_BUTTONF_DOWN) && (buttons & EJ_BUTTONF_RIGHT) )
		return Angle::Revolution( SCALAR_CONSTANT( 0.375 ) );

	if ( (buttons & EJ_BUTTONF_DOWN) && (buttons & EJ_BUTTONF_LEFT) )
		return Angle::Revolution( SCALAR_CONSTANT( 0.125 ) );

	if ( buttons & EJ_BUTTONF_LEFT )
		return Angle::Revolution( Scalar::zero );

	if ( buttons & EJ_BUTTONF_DOWN )
		return Angle::Revolution( SCALAR_CONSTANT( 0.25 ) );

	if ( buttons & EJ_BUTTONF_UP )
		return Angle::Revolution( SCALAR_CONSTANT( 0.75 ) );

	if ( (buttons & EJ_BUTTONF_UP) && (buttons & EJ_BUTTONF_LEFT) )
		return Angle::Revolution( SCALAR_CONSTANT( 0.875 ) );

    return GetPhysicalAttributes().Rotation().GetC();
}

//============================================================================


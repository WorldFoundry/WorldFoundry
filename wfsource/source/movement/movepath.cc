//============================================================================
// movepath.cc: Movement handler for objects on 3D Studio Keyframer Paths
//============================================================================
// Copyright ( c ) 1994,1995,1996,1997,1998,1999,2000,2002,2003 World Foundry Group.  
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
//	History:
//			Created	by William B. Norris IV
//          Updated to support rotation keys, Phil Torre 04/08/97
//============================================================================

#define _MOVEPATH_CC
#include <math/vector3.hp>
#include <movement/movement.hp>
#include <movement/movementobject.hp>
#include <movement/movementmanager.hp>
#include <cpplib/libstrm.hp>
#include <physics/physical.hp>
#include <oas/movement.h>
#include "movepath.hp"

//==============================================================================
                                            
PathHandler			thePathHandler;

// Handlers just for path-based movement
// MovementHandler* pathHandlerTable[] =
// {
//    &thePathHandler,
//    &theNullHandler
// };

//============================================================================

void
PathHandler::init( MovementManager& movementManager, MovementObject& /*movementObject*/ )
{
    assert( movementManager.MovementBlock()->Mobility == MOBILITY_PATH );

//	Path* path = (Path *)getPath( thisActor );
//	assert( ValidPtr( path ) );

//	// Set follow offset if this is a relative path
//	const _Movement* _movementData = thisActor->GetMovementBlockPtr();
//	assert( ValidPtr( _movementData ) );

//	if ( _movementData->ObjectToFollow != -1 )
//	{
//		const PhysicalAttributes& followAttr = theLevel->getActor( _movementData->ObjectToFollow )->GetPhysicalAttributes();
//		const PhysicalAttributes& actorAttr = thisActor->GetPhysicalAttributes();
//		path->_positionOffset = followAttr.Position() - actorAttr.Position();
//		path->_rotationOffset = followAttr.orientation() - actorAttr.orientation();
//	}
}

//=============================================================================

bool
PathHandler::check()
{
	assert(0);
	return true;
}

//=============================================================================

void
PathHandler::predictPosition( MovementManager& movementManager, MovementObject& movementObject, const Clock& clock, const BaseObjectList& baseObjectList )
{                  
	const _Movement* _movementData = movementObject.GetMovementBlockPtr();
	assert( ValidPtr( _movementData ) );

	if ( movementObject.GetMailboxes().ReadMailbox( _movementData->MovementMailbox ).AsBool() )
	{
      Path* path = movementObject.GetPath();
		assert( ValidPtr( path ) );

		PhysicalAttributes& objectPhysicalAttr = movementObject.GetWritablePhysicalAttributes();

		// Add offsets if this is a relative path
		if ( _movementData->ObjectToFollow != -1 )
		{
			// force object to predict it's position
			BaseObject* targetBase = baseObjectList[_movementData->ObjectToFollow];
			AssertMsg(ValidPtr(targetBase),"object to follow (index " << _movementData->ObjectToFollow << ") not found in object " << movementObject);
         MovementObject* target = dynamic_cast<MovementObject*>(targetBase);
         assert(ValidPtr(target));

			if(!target->GetPhysicalAttributes().HasRunPredictPosition())
				target->predictPosition(clock);

			path->SetBase( target->GetPhysicalAttributes().PredictedPosition());
			path->SetBaseRot( target->GetPhysicalAttributes().Rotation() );
		}

		Scalar time = clock.Current();
		Scalar numLoops = time / path->EndTime();
		if (numLoops > Scalar::one)
		{
			Scalar partialTime = time - (numLoops.Floor() * path->EndTime());
         bool numLoopsOdd = numLoops.WholePart() & 1;

			switch (_movementData->AtEndOfPath)
			{
				case AT_END_OF_PATH_PING_PONG:
					if (!numLoopsOdd)
						time = partialTime;
					else
						time = path->EndTime() - partialTime;
					break;

				case AT_END_OF_PATH_STOP:
					time = path->EndTime();
					break;

				case AT_END_OF_PATH_JUMPBACK:
				case AT_END_OF_PATH_WARPBACK:	// jumpback WITHOUT interpolated collision box
					time = partialTime;
					break;

				case AT_END_OF_PATH_DELETE:
					time = path->EndTime();
               movementObject.KillSelf();
					//theLevel->SetPendingRemove( &movementObject );
					break;

				case AT_END_OF_PATH_DERAIL:
					time = path->EndTime();
					movementManager.SetMovementHandler(&theAirHandler,movementObject);
					break;
			}
		}

		Vector3 newPos = path->Position(time);
		Euler newRot = path->Rotation(time);

		DBSTREAM3(cmovement << "setting predicted position to " << newPos << std::endl; )
		DBSTREAM3(cmovement << "setting rotation to " << newRot << std::endl; )

		objectPhysicalAttr.SetPredictedPosition( newPos );
		objectPhysicalAttr.SetRotation( newRot );

#pragma message("WARNING:  Warpback objects won't work with a path less than 0.5 seconds long!!")
#pragma message("          Also, the linear velocity must be constant across the entire path!!")
// I have to look forward on the object's path to make up an imaginary velocity, so I have to have
// somewhere to look forward to.  I know this sucks, but see no alternative without resorting to
// "velocity along path" numbers in the OAD or some such kludge.  Oh, well, the real universe doesn't
// support this feature very well either.
		if (_movementData->AtEndOfPath != AT_END_OF_PATH_WARPBACK)
			objectPhysicalAttr.SetLinVelocity( (newPos - objectPhysicalAttr.Position()) / clock.Delta() );
		else
		{
			AssertMsg(path->EndTime() >= Scalar::half, "Path must be at least 0.5 seconds long");
			objectPhysicalAttr.SetLinVelocity( (path->Position(Scalar::half) - path->Position(Scalar::zero)) / Scalar::half );
		}

	}
}

//=============================================================================

bool
PathHandler::update(MovementManager& /*movementManager*/,  MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
	MsgPort& msgPort = movementObject.GetMsgPort();

   char msgData[msgDataSize];
	while( msgPort.GetMsgByType( MsgPort::COLLISION, &msgData, msgDataSize) )
		;


#if 0
	// kts added 12/5/96 2:04PM
	// if we are moving relative to another object
	// we need to re-calc our position based on the clipped predicted position of the object we are following
	// end kts addition
	if ( _movementData->ObjectToFollow != -1 )
	{ // force object to update

      BaseObject* targetBase = baseObjectList[_movementData->ObjectToFollow];
      AssertMsg(ValidPtr(targetBase),"object to follow (index " << _movementData->ObjectToFollow << ") not found in object " << movementObject);
      MovementObject* target = dynamic_cast<MovementObject*>(targetBase);
      assert(ValidPtr(target));

		if(!target->hasRunUpdate())
			target->update();

		// get pos of object
		const Vector3& targetPos = target->GetPredictedPosition();
		DBSTREAM3(cmovement << "setting base of "
							<< movementObject
							<< " to " << targetPos
							<< std::endl; )
		// store it into base
		path->setBase( targetPos );
	}

#endif


	return true;
}

//=============================================================================

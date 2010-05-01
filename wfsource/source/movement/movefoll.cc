//============================================================================
// movefoll.cc:
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
// Description: movement handler which follows another object
//============================================================================

#define _MOVEFOLL_CC
#include <math/vector3.hp>
#include <physics/physical.hp>
#include <oas/movement.h>
#include "movefoll.hp"

//============================================================================
// Instances of movement handlers
FollowHandler	theFollowHandler;

//============================================================================

void
FollowHandler::init(MovementManager& movementManager, MovementObject& /*movementObject*/ )
{
	assert( movementManager.MovementBlock()->Mobility == MOBILITY_FOLLOW );

#if DO_ASSERTIONS
	FollowHandlerData* followData = (FollowHandlerData*)(movementManager.GetMovementHandlerData());
	assert( ValidPtr( followData ) );
	followData->_idxOwner = -1;
#endif
}

//============================================================================

bool
FollowHandler::check()
{
	assert(0);
	return true;
}

//============================================================================

bool
FollowHandler::update(MovementManager& movementManager,  MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
   char msgData[msgDataSize];

	MsgPort& msgPort = movementObject.GetMsgPort();

	// EAT ALL COLLISION MESSAGES
	while( msgPort.GetMsgByType(MsgPort::COLLISION, &msgData, msgDataSize))		// Receive a message
		;

	FollowHandlerData* followData = (FollowHandlerData*)(movementManager.GetMovementHandlerData());
	assert( ValidPtr( followData ) );

	int32 _owner = followData->_idxOwner;
	assert( _owner > 0 );

   BaseObject* targetBase = baseObjectList[_owner];
   AssertMsg(ValidPtr(targetBase),"owner (index " << _owner << ") not found in object " << movementObject);
   MovementObject* pOwner = dynamic_cast<MovementObject*>(targetBase);
   assert(ValidPtr(pOwner));
   
	if ( !pOwner->GetPhysicalAttributes().HasRunUpdate() )
		pOwner->update();

   // kts 12/8/2002 21:25:58 why was this using PredictedPosition instead of position?
	Vector3 pos = pOwner->GetPhysicalAttributes().PredictedPosition();
	movementObject.GetWritablePhysicalAttributes().SetPredictedPosition( pos );

	return true;
}

//============================================================================

void
FollowHandler::predictPosition( MovementManager& movementManager, MovementObject& /*movementObject*/, const Clock& currentTime, const BaseObjectList& baseObjectList )
{
	FollowHandlerData* followData = (FollowHandlerData*)(movementManager.GetMovementHandlerData());
	assert( ValidPtr( followData ) );

	int32 _owner = followData->_idxOwner;

	assert( _owner > 0 );
   BaseObject* baseObject = baseObjectList[_owner];
   assert(ValidPtr(baseObject));
   MovementObject* mo = dynamic_cast<MovementObject*>(baseObject);
   assert(ValidPtr(mo));

	if ( !mo->GetPhysicalAttributes().HasRunPredictPosition())
      mo->predictPosition(currentTime);

   // kts why doesn't this set the predicted position here?
}

//============================================================================

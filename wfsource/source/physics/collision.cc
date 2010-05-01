//==============================================================================
//	collisio.cc:	New, improved collision system (contains new stuff and stuff
//					pulled out of Room and movement).
//
//	Created 8/5/97 13:36 by Phil Torre
//	Copyright 1997, 2000, 2002 World Foundry Group.  
//==============================================================================

#include "collision.hp"
#include <oas/movement.h>
#include <cpplib/libstrm.hp>
#include <physics/physical.hp>
#include <physics/physicalobject.hp>

//==============================================================================

collisionEvent collisionEventList[MAX_COLLISION_EVENTS];
int collisionEventListLength = 0;
PhysicalObject* recollisionList[MAX_COLLISION_EVENTS];
int recollisionListLength = 0;

//==============================================================================
// Sort the global Collision Event List, in decreasing order of event time
void
SortCollisionEventList(const Clock& clock)
{
	DBSTREAM3( ccollision << "SortCollisionEventList():  Processing " << collisionEventListLength << " events." << std::endl; )
	AssertMsg(collisionEventListLength > 0, "The collision event list is empty; why was I called?");

	{
		for (int eventIndex=0; eventIndex < collisionEventListLength; eventIndex++)
		{
			ComputeCollisionEventTime( collisionEventList[eventIndex], clock );
		}
	}

#pragma message("PERFORMANCE WARNING:  This sort routine really sucks.  Write a better one.")

	collisionEvent tempEvent;
	for (int eventIndex=1; eventIndex < collisionEventListLength; eventIndex++)
	{
		if ( collisionEventList[eventIndex].eventTime > collisionEventList[eventIndex-1].eventTime )
		{
			memcpy(&tempEvent, &collisionEventList[eventIndex], sizeof(collisionEvent));
			memcpy(&collisionEventList[eventIndex], &collisionEventList[eventIndex-1], sizeof(collisionEvent));
			memcpy(&collisionEventList[eventIndex-1], &tempEvent, sizeof(collisionEvent));
			eventIndex = 0;		// will become 1 when loop restarts
		}
	}

	DBSTREAM3(
		ccollision << std::endl << "Sorted collision event list: (" << collisionEventListLength << " events)" << std::endl;
		for (int debugIndex=0; debugIndex < collisionEventListLength; debugIndex++)
		{
			ccollision << "Event #" << debugIndex << ":" << std::endl;
			ccollision << "\tobject1:     " << *collisionEventList[debugIndex].object1 << std::endl;
			ccollision << "\tobject2:     " << *collisionEventList[debugIndex].object2 << std::endl;
			ccollision << "\tEvent Time: " << collisionEventList[debugIndex].eventTime << std::endl;
		}
		ccollision << std::endl;
	)
}

//==============================================================================
//	Given the initial state of two objects whos expanded collision boxes are
//	touching, compute how long it will take for their unexpanded collision
//	boxes to touch.  Also determines the collision normal, which is passed on
//	in the collsionEvent struct.
void
ComputeCollisionEventTime( collisionEvent& thisEvent, const Clock& clock )
{
	const PhysicalAttributes& attr1 = thisEvent.object1->GetPhysicalAttributes();
	const PhysicalAttributes& attr2 = thisEvent.object2->GetPhysicalAttributes();
	attr1.Validate();
	attr2.Validate();
	const ColSpace& colSpace1 = attr1.GetColSpace();
	const ColSpace& colSpace2 = attr2.GetColSpace();
	const Vector3 pos1 = attr1.Position();
	const Vector3 pos2 = attr2.Position();
	bool overlappedInX(false), overlappedInY(false), overlappedInZ(false);

	AssertMsg( !attr1.PremoveCollisionCheck(attr2), *thisEvent.object1 << " and " << *thisEvent.object2 );

#if DO_DEBUGGING_INFO
//	if (attr1.ContainsSlope() || attr2.ContainsSlope())
//		breakpoint();

//	if ( !(attr1.LinVelocity() == Vector3::zero) && !(attr2.LinVelocity() == Vector3::zero) )
//		breakpoint();
#endif

	DBSTREAM3( ccollision << "Computing collision event time:" << std::endl; )
	DBSTREAM3( ccollision << *thisEvent.object1 << std::endl << attr1 << std::endl << std::endl; )
	DBSTREAM3( ccollision << *thisEvent.object2 << std::endl << attr2 << std::endl << std::endl; )

	thisEvent.valid = true;
	thisEvent.normal = Vector3::zero;

	// Consider object1 to be stationary, and assign all velocity to object2
	Vector3 relativeVelocity = attr2.MeanVelocity() - attr1.MeanVelocity();

	// If either object is a slope, check the normal to see if we are hitting the
	// sloped side
	Vector3 slopeNormal = colSpace1.SlopeNormal();
	if (colSpace2.ContainsSlope())
		slopeNormal = colSpace2.SlopeNormal();

	// Compute separation in time between objects in X, Y, and Z.  The LARGEST one determines
	// how long until we hit.
	Scalar separationX(Scalar::zero), separationY(Scalar::zero), separationZ(Scalar::zero);

   Vector3 cs1UMin = colSpace1.UnExpMin(pos1);
   Vector3 cs1UMax = colSpace1.UnExpMax(pos1);
   Vector3 cs2UMin = colSpace2.UnExpMin(pos2);
   Vector3 cs2UMax = colSpace2.UnExpMax(pos2);

	if ( (relativeVelocity.X() < Scalar::zero) &&
		 (cs1UMax.X() <= cs2UMin.X()) &&
		 (slopeNormal.X() == Scalar::zero) )
	{
		separationX = (cs2UMin.X() - cs1UMax.X() - Scalar::epsilon) / -relativeVelocity.X();
		thisEvent.normal = Vector3::unitX;
	}
	else if ( (relativeVelocity.X() > Scalar::zero) &&
			  (cs1UMin.X() >= cs2UMax.X()) &&
			  (slopeNormal.X() == Scalar::zero) )
	{
		separationX = (cs1UMin.X() - cs2UMax.X() - Scalar::epsilon) / relativeVelocity.X();
		thisEvent.normal = Vector3::unitNegativeX;
	}
	else
		overlappedInX = true;

	if ( (relativeVelocity.Y() < Scalar::zero) &&
		 (cs1UMax.Y() <= cs2UMin.Y()) &&
		 (slopeNormal.Y() == Scalar::zero) )
	{
		separationY = (cs2UMin.Y() - cs1UMax.Y() - Scalar::epsilon) / relativeVelocity.Y().Abs();
		thisEvent.normal = Vector3::unitY;
	}
	else if ( (relativeVelocity.Y() > Scalar::zero) &&
			  (cs1UMin.Y() >= cs2UMax.Y()) &&
			  (slopeNormal.Y() == Scalar::zero) )
	{
		separationY = (cs1UMin.Y() - cs2UMax.Y() - Scalar::epsilon) / relativeVelocity.Y().Abs();
		thisEvent.normal = Vector3::unitNegativeY;
	}
	else
		overlappedInY = true;

	if ( (relativeVelocity.Z() < Scalar::zero) &&
		 (cs1UMax.Z() <= cs2UMin.Z()) &&
		 (slopeNormal.Z() == Scalar::zero) )
	{
		separationZ = (cs2UMin.Z() - cs1UMax.Z() - Scalar::epsilon) / relativeVelocity.Z().Abs();
		thisEvent.normal = Vector3::unitZ;
	}
	else if ( (relativeVelocity.Z() > Scalar::zero) &&
			  (cs1UMin.Z() >= cs2UMax.Z()) &&
			  (slopeNormal.Z() == Scalar::zero) )
	{
		separationZ = (cs1UMin.Z() - cs2UMax.Z() - Scalar::epsilon) / relativeVelocity.Z().Abs();
		thisEvent.normal = Vector3::unitNegativeZ;
	}
	else
		overlappedInZ = true;

	// compute distance to any embedded slope, iff we're touching the sloped side
	Scalar separationToSlope(Scalar::zero);
	if ( overlappedInX && overlappedInY && overlappedInZ )
	{
		AssertMsg( (!attr1.ContainsSlope() || !attr2.ContainsSlope()), "I don't know how to collide a slope with a slope yet!");
		if (attr1.ContainsSlope())
		{
			separationToSlope = attr1.TimeToHitSlope(attr2, clock.Delta());
			thisEvent.normal = attr1.SlopeNormal();
		}
		else if (attr2.ContainsSlope())
		{
			separationToSlope = attr2.TimeToHitSlope(attr1, clock.Delta());
			thisEvent.normal = attr2.SlopeNormal() * Scalar::negativeOne;
		}

		if (separationToSlope > clock.Delta())
			thisEvent.valid = false;
		thisEvent.eventTime = separationToSlope;
		return;
	}

	assert(!(thisEvent.normal == Vector3::zero));

	// Return the largest axial separation
	if ( (separationX >= separationY) && (separationX >= separationZ) )
	{
		if (separationX > clock.Delta())
			thisEvent.valid = false;
		thisEvent.eventTime = separationX;
		return;
	}

	if ( (separationY >= separationX) && (separationY >= separationZ) )
	{
		if (separationY > clock.Delta())
			thisEvent.valid = false;
		thisEvent.eventTime = separationY;
		return;
	}

	if (separationZ > clock.Delta())
		thisEvent.valid = false;
	thisEvent.eventTime = separationZ;
}

//============================================================================
// Routine to dispatch messages to two object who are in collision

void
DispatchCollisionMessages( PhysicalObject& object1, PhysicalObject& object2,
	const MsgPort::MSG_TYPE msg1, const MsgPort::MSG_TYPE msg2 )
{
	if ( msg1 && (msg1 != CI_PHYSICS) )
	{
		DBSTREAM3( ccollision << "Sent " << msg1 << " message to " << object1 << " referring to " << object2 << std::endl; )
		object1.sendMsg( msg1, int32(&object2) );
	}

	if ( msg2 && (msg2 != CI_PHYSICS) )
	{
		DBSTREAM3( ccollision << "Sent " << msg2 << " message to " << object2 << " referring to " << object1 << std::endl; )
		object2.sendMsg( msg2, int32(&object1) );
	}
}

//==============================================================================

void
ObjectMustReCollide( PhysicalObject* thisObject )
{
	DBSTREAM3( ccollision << "Invalidating all collisions for " << *thisObject << std::endl; )

	// Put this object in the re-collide list
	recollisionList[recollisionListLength++] = thisObject;

	// Walk the pending collision list and invalidate any events that involve this object
	for (int eventIndex=0; eventIndex < collisionEventListLength; eventIndex++)
	{
		if ( (collisionEventList[eventIndex].object1 == thisObject) ||
			 (collisionEventList[eventIndex].object2 == thisObject) )
			 collisionEventList[eventIndex].valid = false;
	}
}

//==============================================================================

void
ResolveCollisionEvent( collisionEvent& thisEvent, const Clock& clock )
{
   assert(ValidPtr(thisEvent.object1));
   assert(ValidPtr(thisEvent.object2));
	// Don't process this collision if neither object obeys physics
	if ( (thisEvent.object1->GetMovementBlockPtr()->Mobility != MOBILITY_PHYSICS) &&
	     (thisEvent.object2->GetMovementBlockPtr()->Mobility != MOBILITY_PHYSICS) )
		thisEvent.valid = false;

	// Don't process this collision if it's already been invalidated
	if (!thisEvent.valid)
		return;

	PhysicalAttributes& attr1 = thisEvent.object1->GetWritablePhysicalAttributes();
	PhysicalAttributes& attr2 = thisEvent.object2->GetWritablePhysicalAttributes();
	attr1.Validate();
	attr2.Validate();
	const ColSpace& colSpace1 = attr1.GetColSpace();
	const ColSpace& colSpace2 = attr2.GetColSpace();
	Scalar deltaT = clock.Delta();

	// Sanity checks
#if DO_ASSERTIONS || SW_DBSTREAM
	if (attr1.PremoveCollisionCheck(attr2))
	{
	    DBSTREAM3( ccollision << "ResolveCollisionEvent():  FUCKED AGAIN." << std::endl; )
		DBSTREAM3( ccollision << *thisEvent.object1 << std::endl << attr1 << std::endl; )
		DBSTREAM3( ccollision << *thisEvent.object2 << std::endl << attr2 << std::endl; )
		AssertMsg(0, "ResolveCollisionEvent() has failed.");
	}
	assert(thisEvent.eventTime <= deltaT);
	assert(!(thisEvent.normal == Vector3::zero));
#endif

	// Move both objects to the position they will occupy at the actual event time
	attr1.SetPosition( attr1.Position() + (attr1.MeanVelocity() * thisEvent.eventTime) );
	attr2.SetPosition( attr2.Position() + (attr2.MeanVelocity() * thisEvent.eventTime) );

	const Vector3& pos1 = attr1.Position();
	const Vector3& pos2 = attr2.Position();

	DBSTREAM3( ccollision << "ResolveCollisionEvent():  Objects at event time:" << std::endl; )
	DBSTREAM3( ccollision << *thisEvent.object1 << std::endl << attr1 << std::endl; )
	DBSTREAM3( ccollision << *thisEvent.object2 << std::endl << attr2 << std::endl; )
	AssertMsg(!attr1.PremoveCollisionCheck(attr2), "Hey! ComputeCollisionEventTime() lied to me!");

	DBSTREAM3( ccollision << "Collision normal (object1->object2) = " << thisEvent.normal << std::endl; )

   // notify objects of collision (in case they want to do something as a result)
   // currently the only action is updating supportingObject in movementhandlerdata
   thisEvent.object1->Collision(*thisEvent.object2,thisEvent.normal);
   thisEvent.object2->Collision(*thisEvent.object1,-thisEvent.normal);

	// Now that we have the collision normal, compute impulse and modify velocities
	Scalar mass1 = thisEvent.object1->GetMovementBlockPtr()->GetMass();
	Scalar mass2 = thisEvent.object2->GetMovementBlockPtr()->GetMass();
	if (thisEvent.object1->GetMovementBlockPtr()->Mobility != MOBILITY_PHYSICS)
		mass1 = SCALAR_CONSTANT(32767);
	if (thisEvent.object2->GetMovementBlockPtr()->Mobility != MOBILITY_PHYSICS)
		mass2 = SCALAR_CONSTANT(32767);

	DBSTREAM3( ccollision << "object1 mass = ";
			   if (mass1 == SCALAR_CONSTANT(32767))
			       ccollision << "INFINITE" << std::endl;
			   else
			   	   ccollision << mass1 << std::endl; )
	DBSTREAM3( ccollision << "object2 mass = ";
			   if (mass2 == SCALAR_CONSTANT(32767))
			       ccollision << "INFINITE" << std::endl;
			   else
			   	   ccollision << mass2 << std::endl; )

	int32 mobility1 = thisEvent.object1->GetMovementBlockPtr()->Mobility;
	int32 mobility2 = thisEvent.object2->GetMovementBlockPtr()->Mobility;

	// Make up some values to handle infinite mass
	Scalar oneOverMass1(Scalar::zero), oneOverMass2(Scalar::zero), impulseOverMass1(Scalar::zero), impulseOverMass2(Scalar::zero);

	if (mass1 != SCALAR_CONSTANT(32767))
		oneOverMass1 = Scalar::one / mass1;

	if (mass2 != SCALAR_CONSTANT(32767))
		oneOverMass2 = Scalar::one / mass2;


	// Use lesser of the elasticity coefficients of the two bodies
	// 1.0=perfectly elastic, 0.0=perfectly inelastic
	Scalar elasticity;
	if (thisEvent.normal.Z() == Scalar::zero)
		elasticity = thisEvent.object1->GetMovementBlockPtr()->GetHorizontalElasticity().Min(
					 thisEvent.object2->GetMovementBlockPtr()->GetHorizontalElasticity() );
	else
		elasticity = thisEvent.object1->GetMovementBlockPtr()->GetVerticalElasticity().Min(
		 			 thisEvent.object2->GetMovementBlockPtr()->GetVerticalElasticity() );

	DBSTREAM3( ccollision << "Using an elasticity coefficient of " << elasticity << std::endl; )

	// Calculate magnitude of impulse
	Vector3 collisionVelocity1 = attr1.OldLinVelocity() + ( (attr1.LinVelocity() - attr1.OldLinVelocity()) * (thisEvent.eventTime / deltaT) );
	Vector3 collisionVelocity2 = attr2.OldLinVelocity() + ( (attr2.LinVelocity() - attr2.OldLinVelocity()) * (thisEvent.eventTime / deltaT) );
	DBSTREAM3( ccollision << "object1 velocity at collision time: " << collisionVelocity1 << std::endl; )
	DBSTREAM3( ccollision << "object2 velocity at collision time: " << collisionVelocity2 << std::endl; )

	Scalar normalCollisionSpeed = (collisionVelocity1 - collisionVelocity2).DotProduct(thisEvent.normal);
	Scalar impulseNumerator = -(Scalar::one + elasticity) * normalCollisionSpeed;
	Scalar impulseDenominator = oneOverMass1 + oneOverMass2;
	assert(impulseDenominator != Scalar::zero);
	Scalar impulse = (impulseNumerator / impulseDenominator).Abs();

	if (impulse == Scalar::zero)
		impulse = Scalar::one;

	DBSTREAM3( ccollision << "Impulse magnitude = " << impulse << std::endl; )

	if (mass1 != SCALAR_CONSTANT(32767))
		impulseOverMass1 = impulse / mass1;

	if (mass2 != SCALAR_CONSTANT(32767))
		impulseOverMass2 = impulse / mass2;

	// Everything is calculated; now apply the impulses to both objects (if they're moveable)

	if ( (mobility1 != MOBILITY_PATH) && (mobility1 != MOBILITY_ANCHORED) )
	{
		// Check to see if the obstacle can be stepped over
		Scalar StepSize = thisEvent.object1->GetMovementBlockPtr()->GetStepSize();
		if ( (thisEvent.normal.Z() == Scalar::zero) &&
			 ((colSpace1.UnExpMin(pos1).Z() + StepSize) >= colSpace2.UnExpMax(pos2).Z()) )
		{
			DBSTREAM3( ccollision << "object1 stepping over..." << std::endl; )
			attr1.SetPosition( attr1.Position() + Vector3(Scalar::zero, Scalar::zero,
							   (colSpace2.UnExpMax(pos2).Z() - colSpace1.UnExpMin(pos1).Z() + Scalar::epsilon) ) );
		}
		else
		{
			DBSTREAM3( ccollision << "object1 velocity modifier = (" << (-thisEvent.normal * impulseOverMass1) << std::endl; )

			Vector3 linVelNormalComponent = thisEvent.normal * attr2.LinVelocity().DotProduct(thisEvent.normal);
			Vector3 colVelNormalComponent = thisEvent.normal * collisionVelocity2.DotProduct(thisEvent.normal);

			// Set LinVelocity to collisionVelocity, along surface normal ONLY
			attr2.AddLinVelocity( linVelNormalComponent );
			attr2.AddLinVelocity( colVelNormalComponent * Scalar::negativeOne );

			attr1.AddLinVelocity( -(thisEvent.normal * impulseOverMass1) );
			attr1.SetOldLinVelocity(attr1.LinVelocity());
		}
		ObjectMustReCollide(thisEvent.object1);
	}

	// Integrate PredictedPosition forward for remainder of frame
	attr1.SetPredictedPosition( attr1.Position() + (attr1.MeanVelocity() * (deltaT - thisEvent.eventTime) ) );
	DBSTREAM3( ccollision << "object1 after collision:" << std::endl << attr1 << std::endl; )

	if ( (mobility2 != MOBILITY_PATH) && (mobility2 != MOBILITY_ANCHORED) )
	{
		// Check to see if the obstacle can be stepped over
		Scalar StepSize = thisEvent.object2->GetMovementBlockPtr()->GetStepSize();
		if ( (thisEvent.normal.Z() == Scalar::zero) &&
			 ((colSpace2.UnExpMin(pos2).Z() + StepSize) >= colSpace1.UnExpMax(pos1).Z()) )
		{
			DBSTREAM3( ccollision << "object2 stepping over..." << std::endl; )
			attr2.SetPosition( attr2.Position() + Vector3(Scalar::zero, Scalar::zero,
							   (colSpace1.UnExpMax(pos1).Z() - colSpace2.UnExpMin(pos2).Z() + Scalar::epsilon) ) );
		}
		else
		{
			DBSTREAM3( ccollision << "object2 velocity modifier = (" << (thisEvent.normal * impulseOverMass2) << std::endl; )

//			attr2.SetLinVelocity(collisionVelocity2);

			Vector3 linVelNormalComponent = thisEvent.normal * attr2.LinVelocity().DotProduct(thisEvent.normal);
			Vector3 colVelNormalComponent = thisEvent.normal * collisionVelocity2.DotProduct(thisEvent.normal);

			// Set LinVelocity to collisionVelocity, along surface normal ONLY
			attr2.AddLinVelocity( linVelNormalComponent * Scalar::negativeOne );
			attr2.AddLinVelocity( colVelNormalComponent );

			attr2.AddLinVelocity( thisEvent.normal * impulseOverMass2 );
			attr2.SetOldLinVelocity(attr2.LinVelocity());
		}
		ObjectMustReCollide(thisEvent.object2);
	}

	// Integrate PredictedPosition forward for remainder of frame
	attr2.SetPredictedPosition( attr2.Position() + (attr2.MeanVelocity() * (deltaT - thisEvent.eventTime) ) );
	DBSTREAM3( ccollision << "object2 after collision:" << std::endl << attr2 << std::endl; )

	thisEvent.valid = false;
}

//============================================================================
// possible ways to avoid a collision check:
//		both objects have a mass of zero
//		both objects are not movable
//		both objects are on paths
//		both objects havn't moved this frame (predicted motion vector = 0)
//		look in CI table

typedef int32 collisionInteraction;

#define CI_NOTHING 0
#define CI_PHYSICS MsgPort::COLLISION
#define CI_SPECIAL MsgPort::SPECIAL_COLLISION

#include "oas/objects.car"

extern collisionEvent collisionEventList[];
extern int collisionEventListLength;

//==============================================================================
// checkObject =  object to check all other object in the room against
// startingObject = # of objects in the room to skip

void 
CollideObjectWithList(PhysicalObject& checkObject, BaseObjectIteratorWrapper poIter, const Clock& clock)
{
	DBSTREAM3( ccollision << "Room::checkCollision: object " << checkObject << ", colbox of " << std::endl; )
	DBSTREAM3( ccollision << "Min " << checkObject.GetPhysicalAttributes().Min() << std::endl; )
	DBSTREAM3( ccollision << "Max : " << checkObject.GetPhysicalAttributes().Max() << std::endl; )

 	register PhysicalObject* pObject;
	while(!poIter.Empty())	// iterate through all objects in this room
	{
		pObject = dynamic_cast<PhysicalObject*>(&(*poIter));
      assert(ValidPtr(pObject));

		if( &checkObject != pObject )
		{
			collisionInteraction ci = collisionInteractionTable[pObject->GetMovementBlockPtr()->MovementClass][checkObject.GetMovementBlockPtr()->MovementClass];

			// Prevent pathed objects from colliding with each other and anchored objects
			if ( (pObject->GetMovementBlockPtr()->Mobility == MOBILITY_PATH) &&
				 (checkObject.GetMovementBlockPtr()->Mobility == MOBILITY_PATH) )
				ci = 0;
			if ( (pObject->GetMovementBlockPtr()->Mobility == MOBILITY_PATH) &&
				 (checkObject.GetMovementBlockPtr()->Mobility == MOBILITY_ANCHORED) )
				ci = 0;
			if ( (pObject->GetMovementBlockPtr()->Mobility == MOBILITY_ANCHORED) &&
				 (checkObject.GetMovementBlockPtr()->Mobility == MOBILITY_PATH) )
				ci = 0;

			// kts if collisionInteraction and at least on of the objects is not anchored
			if ( ci  && (pObject->GetMovementBlockPtr()->Mobility ||  checkObject.GetMovementBlockPtr()->Mobility ))
			{
                if( checkObject.GetPhysicalAttributes().FullCollisionCheck(pObject->GetPhysicalAttributes(),clock ))
                {
                    // Send messages if special collision, or add to event list if physics collision
                    if ( ((ci>>16) == CI_SPECIAL) || ((ci & 0xFFFF) == CI_SPECIAL) )
                        DispatchCollisionMessages( checkObject, *pObject,
                            MsgPort::MSG_TYPE(ci>>16), MsgPort::MSG_TYPE(ci & 0xFFFF) );

                    if ( ((ci>>16) == CI_PHYSICS) || ((ci & 0xFFFF) == CI_PHYSICS) )
                    {
                        collisionEventList[collisionEventListLength].object1 = &checkObject;
                        collisionEventList[collisionEventListLength].object2 = pObject;
                        collisionEventList[collisionEventListLength++].eventTime = Scalar::zero;
                        AssertMsg(collisionEventListLength < MAX_COLLISION_EVENTS, "The collision event list is full!");
                    }
                }
			}
		}
		++poIter;
	}
}

//==============================================================================


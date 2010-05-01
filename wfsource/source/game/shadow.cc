//============================================================================
// Shadow.cc:
// Copyright ( c ) 1995,1996,1997,1999,2000,2001,2002,2003 World Foundry Group  
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
//
// TODO:
//		CheckBelowOnlyOnce not implemented correctly and not verified
//
//============================================================================

#include <oas/shadow.ht>
#include <room/actroit.hp>
#include <movement/movementobject.hp>
#include "shadow.hp"
#include "level.hp"
extern NullHandler		theNullHandler;

//============================================================================

Shadow::Shadow( const SObjectStartupData* startupData ) : Actor( startupData )
{
	_floor = NULL;
   GetMovementManager().SetMovementHandler(&theNullHandler,*this);
	_shadowedObject = 0;

	assert( CanRender() );
}

//============================================================================

Actor::EActorKind
Shadow::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Shadow_KIND);
	return Actor::Shadow_KIND;
}

//============================================================================

void
Shadow::setShadowPos( const PhysicalObject& target )
{
	AssertMsg( _shadowedObject > 0, *this << ": No object to shadow defined" << std::endl );
	assert( _floor );
//	if ( _floor )
	{   // update shadow to correct floor height
		assert( _floor != &target );
		Vector3 pos = target.GetPhysicalAttributes().PredictedPosition();
		pos.SetZ( _floor->GetPhysicalAttributes().Max().Z() + SCALAR_CONSTANT(0.01) );
		GetWritablePhysicalAttributes().SetPredictedPosition( pos );
      Euler targetEuler = target.GetPhysicalAttributes().Rotation();
      Euler euler = GetPhysicalAttributes().Rotation();
      euler.SetC(targetEuler.GetC());
		GetWritablePhysicalAttributes().SetRotation(euler);
	}
}

//==============================================================================
// iterate through all objects looking for the first object under this one which can be stood on
// this must be really slow

BaseObject*
findFloor( const BaseObject& object ) 
{
   const PhysicalObject& po = UpcastToPhysicalObject(&object);
	const PhysicalAttributes& actPa =  UpcastToPhysicalObject(&object).GetPhysicalAttributes();
	const ColSpace& actCs = actPa.GetColSpace();
	const Vector3& actPos = actPa.Position();

	PhysicalObject* belowObj = NULL;

#if 1
	ActiveRoomsIterConst roomIter(theLevel->GetActiveRooms().GetIter());

	while(!roomIter.Empty())
	{
//		if ( !_theActiveRooms.GetActiveRoom(roomIndex) )
//			continue;

		if ( (*roomIter).CheckCollision(po) )
		{ // in this room
			for ( BaseObjectIteratorWrapper poIter = (*roomIter).ListIter(ROOM_OBJECT_LIST_RENDER) ; !poIter.Empty(); ++poIter )
		 	{
	  		   PhysicalObject& obj = UpcastToPhysicalObject(&(*poIter));

				//std::cout << "Shadow checking " << *actor << " vs. " << obj << std::endl;
				const PhysicalAttributes& objPa = obj.GetPhysicalAttributes();
				if ( actCs.CheckBelow( actPos, objPa.GetColSpace(), objPa.Position() ))
				{
					if ( (belowObj == NULL ) || ( belowObj->GetPhysicalAttributes().Max().Z() <= objPa.Max().Z() ) )
					{
						if ( ( obj.kind() != BaseObject::Shadow_KIND ) && ( &obj != &po ) )
						{
							belowObj = &obj;
							//std::cout << "Setting belowObj to " << *belowObj << std::endl;
						}
					}
				}
			}
			break;
		}
	}
#else
#pragma message ("KTS: make this stop after it finds something in the current room")
	ActiveRoomsBaseObjectIter arpoi = GetActiveRooms().GetRenderPhysicalObjectIter();

	while(arpoi.Empty())
	 {
      PhysicalObject& obj = *arpoi;

		if
		(
			( actCs.CheckBelow( actPos, obj.GetColSpace(), obj.Position() ))
			&& ( (belowObj == 0 ) || ( belowObj->GetPhysicalAttributes().MaxZ() < obj.MaxZ() ))
		)
         // kts lame, instead of checking for shadow, should check for mass, which is a much
         // better way of eliminating objects which you can't stand on
			if ( ( obj.kind() != Actor::Shadow_KIND ) && ( &obj != actor ) )
				belowObj = &obj;

		++arpoi;
	 }
#endif
	return belowObj;
}

//==============================================================================

void
Shadow::predictPosition(const Clock& currentTime)
{
	AssertMsg( _shadowedObject > 0, *this << ": No object to shadow defined" << std::endl );
	if ( !hasRunPredictPosition() )
	{
		GetWritablePhysicalAttributes().HasRunPredictPosition(true);

		BaseObject* bo = theLevel->GetObject( _shadowedObject );

		if ( bo )
		{
         MovementObject& target = UpcastToMovementObject(bo);

			if ( !target.GetPhysicalAttributes().HasRunPredictPosition() )
				target.predictPosition(currentTime);

			BaseObject* floorBo = findFloor( target );
         
			if ( floorBo )
         {
            _floor = dynamic_cast<PhysicalObject*>(floorBo);
            setShadowPos( target );
         }
         else
            _floor = NULL;
		}
		else
			_floor = NULL;
	}
}

//============================================================================

void
Shadow::SetShadowedObject( int32 idxActor )
{
	_shadowedObject = idxActor;
	AssertMsg( _shadowedObject > 0, *this << ": No object to shadow defined" << std::endl );
}

//============================================================================

bool
Shadow::isVisible() const
{
	return bool( _floor ) && Actor::isVisible();
}

//============================================================================

void
Shadow::update()
{
	BaseObject* baseObject = theLevel->GetObject( _shadowedObject );
	if ( baseObject )
	{
      MovementObject& mo = UpcastToMovementObject(baseObject);

		if ( !mo.GetPhysicalAttributes().HasRunUpdate() )
			mo.update();

		Actor::update();

		setCurrentPos( mo.GetPhysicalAttributes().Position() );
	}
	else
	{	// If owner went away, then we should, too
		theLevel->SetPendingRemove( this );
	}
}

//============================================================================

Actor*
OadShadow( const SObjectStartupData* startupData )
{
	return new (*startupData->memory) Shadow( startupData );
}

//============================================================================

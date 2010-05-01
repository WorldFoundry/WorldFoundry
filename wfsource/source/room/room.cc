//=============================================================================
// Room.cc:
// Copyright ( c ) 1996,1997,1999,2000,2001,2002,2003 World Foundry Group  
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

// ===========================================================================
//	Abstract: Room class, rooms are contained in world, and contains a tree of objects
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

#define _ROOM_CC
#include <cpplib/libstrm.hp>
#include <oas/levelobj.ht>
#include <oas/room.ht>
#include <baseobject/msgport.hp>
#include <physics/collision.hp>
#include <asset/assslot.hp>
#include "room.hp"
#include "rooms.hp"

//==============================================================================

void  
RoomCallbacks::Validate() const
{
#if DO_VALIDATION
   _Validate();
#endif   
}
                                  
//============================================================================

Room::Room() :
	_physicalAttributes()
{
	for( int roomIndex=0; roomIndex < MAX_ACTIVE_ROOMS; ++roomIndex )
		adjacentRooms[ roomIndex ] = 0;
}

//============================================================================

Room::~Room()
{
   Validate();
   delete [] _objectLists;
}

//============================================================================
// this is done this way so that level can allocate an array of rooms at once
// kts: why not use placement new to construct them?
// kts: for that matter, now that we have LMalloc, who cares if there is more than one allocation

void
Room::Construct
(
	_RoomOnDisk* roomData,
	int maxObjects,
	Room* levelRooms,
	int levelNumRooms,
	int roomIndex,
   Memory& memory,
   int numberOfTemporaryObjects,
   PRoomObjectListCheckFunc* listCheckFuncList,
   int checkListEntries,
   Array<BaseObject*>& masterObjectList, 
   RoomCallbacks* roomCallbacks
)
{
   assert(ValidPtr(roomData));
   (void)maxObjects;
   RangeCheck(1,maxObjects,10000);               // arbitrary
   assert(ValidPtr(levelRooms));
   RangeCheck(1,levelNumRooms, 1000);             // arbitrary
   RangeCheck(0,roomIndex,1000);                      // arbitrary
   memory.Validate();
   RangeCheck(0,numberOfTemporaryObjects,1000);  // arbitrary
   assert(ValidPtr(listCheckFuncList));
   RangeCheck(0,checkListEntries,100);           // arbitrary
   assert(ValidPtr(roomCallbacks));
   masterObjectList.Validate();

	Int16List _allObjects;
   
   _listCheckFuncList = listCheckFuncList;
   _checkListEntries = checkListEntries;
   _roomCallbacks = roomCallbacks;
   _masterObjectList = &masterObjectList;
   assert(ValidPtr(_masterObjectList));
   _objectLists = new (memory) Int16List[_checkListEntries];

	DBSTREAM1( croom << "Room::Construct: roomIndex  = " << roomIndex << std::endl; )

   const struct _CollisionRectOnDisk* cod =  &roomData->boundingBox;
   Vector3 min( Scalar::FromFixed32( cod->minX ), Scalar::FromFixed32( cod->minY ), Scalar::FromFixed32( cod->minZ ) );
   Vector3 max( Scalar::FromFixed32( cod->maxX ), Scalar::FromFixed32( cod->maxY ), Scalar::FromFixed32( cod->maxZ ) );
	_physicalAttributes.Construct( Vector3::zero, Euler::zero, min, max );

	_roomIndex = roomIndex;

	// read in objects in the room
	_allObjects.Construct(memory, roomData->count);

	DBSTREAM1( croom << "  Creating allobjects array" << std::endl; )
	_RoomOnDiskEntry* objectIndexArray = (_RoomOnDiskEntry*)((char*)roomData + sizeof(_RoomOnDisk));
	int entry;
	for( entry = 0; entry < roomData->count; ++entry )
	{
      DBSTREAM1( croom << "   checking array entry " << entry << ": object # " << objectIndexArray[entry].object << std::endl; )
		RangeCheck(0,objectIndexArray[entry].object,maxObjects);

		if((*_masterObjectList)[objectIndexArray[entry].object] != NULL)
		 {
			_allObjects.Add(objectIndexArray[entry].object);
			DBSTREAM1( croom << "    adding object # " << objectIndexArray[entry].object << " to room object index " << entry << std::endl; )
		 }
	}
	AssertMsg(_allObjects.Size() > 0,"Room contains no objects");
	adjacentRooms[0] = this;

		// create list of rooms to be active when this one is
		// the current room is always in index 0, followed by the others
	DBSTREAM1( croom << "  Creating adjacent room list" << std::endl; )
	for (entry = 0; entry < MAX_ADJACENT_ROOMS; entry++)
	 {
		int roomnum = roomData->adjacentRooms[entry];
		if (roomnum == ADJACENT_ROOM_NULL)
		 {
			adjacentRooms[entry+1] = NULL;
		 }
		else
		 {
			(void)levelNumRooms;
			RangeCheck(0,roomnum,levelNumRooms);
			adjacentRooms[entry+1] = &levelRooms[roomnum];
		 }
	 }

	// initialize lists of objects in room
   RangeCheck(0,_checkListEntries, 20);         // kts if this fires just increase the size of the following array
   int maxListEntries[20];

   for(int index=0;index<_checkListEntries;index++)
   {
      maxListEntries[index] = 0;
   }

	// FIX - read in number of various lists from level data file
	// calculate size of each array
	DBSTREAM1( croom << "  counting # of objects qualify for each list" << std::endl; )

   BaseObjectIteratorFromInt16List allIter(_allObjects,*_masterObjectList);
	while(!allIter.Empty())
	 {
		BaseObject& object = *allIter;
      for(int listIndex=0;listIndex<_checkListEntries;listIndex++)
      {
         PRoomObjectListCheckFunc func = _listCheckFuncList[listIndex];
         if((object.*func)())
            maxListEntries[listIndex]++;
      }
		++allIter;
	 }
	 // kts this is not really right, since objects can move between rooms
   for(int listIndex=0;listIndex<_checkListEntries;listIndex++)
   {
      _objectLists[listIndex].Construct(memory,maxListEntries[listIndex]+numberOfTemporaryObjects);
   }

	//AssertMsg( maxListEntries[ROOM_OBJECT_LIST_ACTIVATION_BOX] > 0, "No activation boxes" );

	// from reset
	DBSTREAM1( croom << "Room::Construct:Creating render and update array, numObjects = " << _allObjects.Size() << std::endl; )
	Int16ListIter allObjectsIter(_allObjects);
	while(!allObjectsIter.Empty())
	 {
		assert(*allObjectsIter >= 0);
		BaseObject* object = (*_masterObjectList)[*allObjectsIter];
		if (object)
		 {
 			DBSTREAM1( croom << "checking object <" << *allObjectsIter <<">, object->kind() = " << object->kind() << std::endl; )

         for(int listIndex=0;listIndex<_checkListEntries;listIndex++)
         {
            PRoomObjectListCheckFunc func = _listCheckFuncList[listIndex];
            if((object->*func)())
            {
               DBSTREAM1( croom << "adding object <" << *allObjectsIter <<"> to objectList[" << listIndex << "]" << std::endl; )
               _objectLists[listIndex].Add(*allObjectsIter);

            }
         }
		 }
	++allObjectsIter;
	 }
	DBSTREAM1( croom << "  Done Creating render and update array" << std::endl; )
}

//==============================================================================

void
Room::BindAssets()
{
#if 0
   int assetSlot = _roomCallbacks->GetSlotIndex(_roomIndex);
	const CCYC& CCYClist = theLevel->GetAssetManager().GetAssetSlot( assetSlot ).GetCCYC();
	assert( ValidPtr( &CCYClist ) );
	DBSTREAM3( std::cout << "&CCYClist = " << &CCYClist << std::endl;
	std::cout << CCYClist << std::endl; )
	//_nColourCycles = CCYClist.NumEntries();
	for ( int idxColourCycle=0; idxColourCycle<_nColourCycles; ++idxColourCycle )
	{
		_CCYC* ccyc = CCYClist.GetCCYC( idxColourCycle );
		assert( ccyc );
		//std::cout << *ccyc << std::endl;
	}
#endif
}

//============================================================================

void
Room::UnBindAssets()
{
}

//============================================================================
// loop through all objects in this room, checking to see if any of them have left this room
// KTS(how about if we only check objects which have MOVED!)
// KTS(how about if we check objects WHEN they move!, then this routine doesn't need to exist)

void
Room::UpdateRoomContents(int updateIndex, LevelRooms& levelRooms)
{
	DBSTREAM2( croom << "Room::updateRoomContents" << std::endl; )

//		DBSTREAM1( croom << "Room::updateRoomContents" << std::endl; )
   
   Int16ListIter alIter(_objectLists[updateIndex] );
	while(!alIter.Empty())
	 {
		BaseObject* object = (*_masterObjectList)[*alIter];
      PhysicalObject* po = dynamic_cast<PhysicalObject*>(object);
      assert(ValidPtr(po));
		if (!CheckCollision(*po))
		 {
			int32 objectIndex = *alIter;
			RemoveObject(objectIndex);

         Validate();
         levelRooms.AddObjectToRoom(objectIndex);
		 }
		++alIter;
	 }
	DBSTREAM3( croom << "Room::updateRoomContents finished" << std::endl; )
}

//============================================================================

void
Room::AddObject( int32 objectIndex )
{
	DBSTREAM1( croom << "R::AA: Adding object " << objectIndex << " to room #" << _roomIndex << std::endl; )
	assert( objectIndex > 0 );

	BaseObject* object = (*_masterObjectList)[ objectIndex ];
	assert( ValidPtr( object ) );

   for(int listIndex=0;listIndex<_checkListEntries;listIndex++)
   {
      PRoomObjectListCheckFunc func = _listCheckFuncList[listIndex];
      if((object->*func)())
      {
         DBSTREAM3( croom << "adding object <" << *object <<"> to objectList[" << listIndex << "] with a kind of " << object->kind() << std::endl; )
         _objectLists[listIndex].Add(objectIndex);
      }
   }
}

//============================================================================

void
Room::RemoveObject(int32 objectIndex)
{
	DBSTREAM1( croom << "R::RA: Removing object " << objectIndex << " from room #" << _roomIndex << std::endl; )
	assert(objectIndex);
	BaseObject* object = (*_masterObjectList)[objectIndex];
	assert(object);

   for(int listIndex=0;listIndex<_checkListEntries;listIndex++)
   {
      PRoomObjectListCheckFunc func = _listCheckFuncList[listIndex];
      if((object->*func)())
         _objectLists[listIndex].Remove(objectIndex);
#if DO_ASSERTIONS
	else
		assert(!_objectLists[listIndex].IsInList(objectIndex));
#endif
   }
}

//==============================================================================

void
Room::CheckCollisionWithObjects(PhysicalObject& checkObject, const Clock& clock, int listIndex, int startingObject) const
{
	DBSTREAM2( croom << "Room::CheckCollisionWithObjects: " << std::endl; )
   BaseObjectIteratorWrapper poIter = ListIter(listIndex);
	poIter += startingObject;
   if(!poIter.Empty())
      CollideObjectWithList(checkObject, poIter, clock);
}

//==============================================================================
// this does level-wide collision detection, this should use the new update tree
//==============================================================================

void
CheckSameRoom( BaseObjectIteratorWrapper boIter, const Room& room, const Clock& clock, int listIndex )
{
	// check a room against a collision list
	int count=1;				// start at 1 so we never check against ourself
	while( !boIter.Empty() )										// iterate through all objects in this room
	{
//		DBSTREAM2( clevel << "QL::dC:CR: cIter loop" << std::endl; )
      PhysicalObject* po = dynamic_cast<PhysicalObject*>(&(*boIter));
      assert(ValidPtr(po));
		room.CheckCollisionWithObjects( *po,clock, listIndex, count );
		++boIter;
		++count;
	}
}

// kts new collision loop, should be more efficent
//==============================================================================
// check collisions against some other room the object is not in

void
CheckOverlappedRoom( BaseObjectIteratorWrapper boIter, const Room& room, const Clock& clock, int listIndex )
{
	// check a room against a collision list
	while( !boIter.Empty() )										// iterate through all objects in this room
	{
//		DBSTREAM2( clevel << "QL::dC:CR: cIter loop" << std::endl; )

      PhysicalObject* po = dynamic_cast<PhysicalObject*>(&(*boIter));
      assert(ValidPtr(po));

		if ( room.CheckCollision( *po ) )
      {
			room.CheckCollisionWithObjects( *po, clock, listIndex );
      }
		++boIter;
	}
}

//==============================================================================

RoomCallbacks::~RoomCallbacks()
{

}

//==============================================================================


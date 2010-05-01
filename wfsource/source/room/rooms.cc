//==============================================================================
// rooms.cc: LevelRooms.cc
// Copyright (c) 1997,1998,1999,2000,2001,2002 World Foundry Group.  
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

#include <physics/physicalobject.hp>
#include "rooms.hp"
#include <cpplib/libstrm.hp>
#include <oas/levelobj.ht>

//==============================================================================

LevelRooms::LevelRooms() :
	_rooms(NULL),
	_numRooms(0),				        // indicate not intialized
	_roomSlotMap( NULL ),
   _roomCallbacks(NULL),
   _roomArray(NULL)
{
}

//=============================================================================

LevelRooms::~LevelRooms()
{
	Validate();

	DBSTREAM1( cprogress << "~LevelRooms:: deleting rooms" << std::endl; )
	MEMORY_DELETE_ARRAY(HALLmalloc,_rooms,Room,_numRooms);
	HALLmalloc.Free(_roomSlotMap, sizeof(int)*_numRooms);
}

//==============================================================================
// construct all of the rooms in the level

void
LevelRooms::InitRooms(int numRooms,	_LevelOnDisk* levelData, PRoomObjectListCheckFunc* listCheckFuncList, Array<BaseObject*>& masterObjectList, RoomCallbacks* roomCallbacks, Memory& memory, const _LevelObj& levelOad)
{
	AssertMsg( numRooms > 0, "No rooms in level" );
   assert(ValidPtr(levelData));
   assert(ValidPtr(listCheckFuncList));
   assert(ValidPtr(roomCallbacks));
   roomCallbacks->Validate();
   memory.Validate();

   _roomCallbacks = roomCallbacks;
   _levelData = levelData;
   _masterObjectList = &masterObjectList;

	_numRooms = 1;

	_roomSlotMap = new (HALLmalloc) int[ numRooms];
	ValidatePtr( _roomSlotMap );

	// read in rooms data
	DBSTREAM1( cprogress << "Reading rooms"  << std::endl; )
	_roomArray = ( int32 * )( (char * )_levelData + ( _levelData->roomsOffset ));
	AssertMsg( numRooms > 0, "No rooms in level" );

	_rooms = new (HALLmalloc) Room[numRooms];

	assert( ValidPtr( _rooms ) );

	for( int index = 0; index < numRooms; index++ )
	{
		_RoomOnDisk * roomData = ( _RoomOnDisk * )( (( char * )_levelData ) + _roomArray[index] );
      
      int checkListEntries = 5;

		_rooms[index].Construct( roomData,_levelData->objectCount,_rooms,numRooms,index, memory,levelOad.NumberOfTemporaryObjects,listCheckFuncList,checkListEntries,masterObjectList, _roomCallbacks  );				
      _rooms[index].Validate();
      _numRooms++;                  // kts kludge so that objects constructed inside of the room don't crash when they iterate all of the rooms to see which they are in
                                    // (only occurs when an object is not in the room it is supposed to be in)
	}

	_numRooms = numRooms;
	InitRoomSlotMap();
	Validate();
}

//==============================================================================

Room* 
LevelRooms::FindContainingRoom(const ColSpace& object, const Vector3& objpos) const
{
   Validate();
	for( int index = 0; index < _numRooms; index++ )
	{
      _rooms[index].Validate();
      if(_rooms[index].CheckCollision( object, objpos))
         return &_rooms[index];
	}
   return NULL;   
}

//==============================================================================

#if 0
Room* 
LevelRooms::FindContainingRoom(const PhysicalObject& object) const
{
   Validate();
   return FindContainingRoom(object.GetPhysicalAttributes().GetColSpace(),object.GetPhysicalAttributes().Position());
}
#endif

//==============================================================================
// this adds an object to whatever room it is in

void
LevelRooms::AddObjectToRoom( int32 objectIndex )
{
	Validate();
	assert( objectIndex >= 0 );
	BaseObject* object = (*_masterObjectList)[objectIndex];
   assert(ValidPtr(object));
	AssertMsg( object->kind() != BaseObject::StatPlat_KIND, "Cannot generate or move a statplat at runtime" );

   PhysicalObject* po = dynamic_cast<PhysicalObject*>(object);
   assert(ValidPtr(po));

	int roomnum = -1;
	// figure out which room to add to
	assert( ValidPtr( _rooms ) );
	for ( int roomIndex = 0; roomIndex < _numRooms; roomIndex++ )
	{
		if ( _rooms[roomIndex].CheckCollision( *po ) )
		{
			roomnum = roomIndex;
			break;
		}
	}

	DBSTREAM1( if ( roomnum < 0 )
					cerror << * object << " is not in any room (or is in the wrong room at startup)" << std::endl );
// kts: can't do this check any more since rooms.cc no longer knows about actors
// it would be nice to have this, though, so we should try to find a way to make it happen
#if 0 //DO_ASSERTIONS
	// make sure that the object is permanent or in the room corresponding
	// to its asset

   Actor* actor = dynamic_cast<Actor*>(object);
   assert(ValidPtr(actor));
	if ( actor->HasMesh() )
	{
		packedAssetID assetID = actor->GetMeshName();
		AssertMsg( assetID.Room() == AssetManager::ROOM_PERM_INDEX ||
			assetID.Room() == roomnum, "Attempted to add non-permanent " << *object
			<< " to room " << roomnum << "." << std::endl
			<< "It started in room " << assetID.Room() << std::endl );
	}
#endif // DO_ASSERTIONS

	if ( roomnum < 0 )
	{
      _roomCallbacks->SetPendingRemove( object );  // left all rooms, kill it
      DBSTREAM1( croom << "LR::AATR: Removing object " << objectIndex << " from game since not in any room" << std::endl; )
}
	else
		_rooms[roomnum].AddObject( objectIndex );
}

//=============================================================================
// remove an object from whatever room it's in

void
LevelRooms::RemoveObjectFromRoom( int32 idxObject )
{
	Validate();
	assert(idxObject > 0);
	BaseObject* object = (*_masterObjectList)[idxObject];
	assert( ValidPtr( object ) );
   PhysicalObject* po = dynamic_cast<PhysicalObject*>(object);
   assert(ValidPtr(po));
	int roomnum = -1;

	assert( ValidPtr( _rooms ) );
	// figure out which room to remove from
	int roomIndex = 0;
	for ( roomIndex = 0; roomIndex < _numRooms; roomIndex++ )
	{
		if ( _rooms[roomIndex].CheckCollision( *po ) )
		{
			roomnum = roomIndex;
			break;
		}
	}

//	AssertMsg( roomnum >= 0, *( _actors[ idxObject ] ) << " is not in any room" << std::endl );
	if ( roomnum >= 0 )
		_rooms[roomIndex].RemoveObject( idxObject );
	else
		_roomCallbacks->SetPendingRemove( object );
}

//==============================================================================
// return ADJACENT_ROOM_NULL if no adjacent

int
LevelRooms::AdjacentRoomIndex( const int roomidx, const int index )
{
	assert( roomidx >= 0 );
	assert( roomidx < _numRooms );
	assert( index >= 0 );
	assert( index < MAX_ADJACENT_ROOMS );

	_RoomOnDisk * roomData = ( _RoomOnDisk * )( ( ( char * )_levelData ) + _roomArray[roomidx] );

	int adjacent = roomData->adjacentRooms[index];

	assert( adjacent == ADJACENT_ROOM_NULL || ( adjacent >= 0 && adjacent < _numRooms ) );

	return adjacent;
}

//=============================================================================

void
LevelRooms::InitRoomSlotMap()
{
	assert(_numRooms > 0);
	// unmark all items
	int idxRoom;
	for( idxRoom = 0; idxRoom < _numRooms; ++idxRoom )
	{
		_roomSlotMap[idxRoom] = ADJACENT_ROOM_NULL;
	}

	// choose an unmarked item
	for( idxRoom = 0; idxRoom < _numRooms; ++idxRoom )
	{
		if( _roomSlotMap[idxRoom] == ADJACENT_ROOM_NULL )
		{
			DBSTREAM1( cstats << "Walking from " << idxRoom << std::endl; )

			int curRoom, lastRoom;
			int slot = 0;

			// mark this room
			_roomSlotMap[idxRoom] = slot;
			DBSTREAM1( cstats << idxRoom << "->" << slot << std::endl; )

			// walk forward
			curRoom = AdjacentRoomIndex( idxRoom, 1 );
			slot = 1;
			while( curRoom != ADJACENT_ROOM_NULL )
			{
				DBSTREAM1( cstats << "forward" << std::endl; )
				AssertMsg( _roomSlotMap[curRoom] == ADJACENT_ROOM_NULL,
					"_roomSlotMap[" << curRoom << "] == " << _roomSlotMap[curRoom] << std::endl );

				// mark
				_roomSlotMap[curRoom] = slot;
				DBSTREAM1( cstats << curRoom << "->" << slot << std::endl; )

				++slot;
				if( slot >= MAX_ACTIVE_ROOMS )
					slot = 0;

				int idxStartRoom = curRoom;
				for( int idxAdjRoom = 0; idxAdjRoom < MAX_ADJACENT_ROOMS; ++idxAdjRoom )
				{
					curRoom = AdjacentRoomIndex( idxStartRoom, idxAdjRoom );
					if( ( curRoom != ADJACENT_ROOM_NULL ) && ( _roomSlotMap[curRoom] == ADJACENT_ROOM_NULL ) )
					{
						break;
					}
					else
					{
						curRoom = ADJACENT_ROOM_NULL;
					}
				}
			}

			// walk backward
			lastRoom = idxRoom;
			curRoom = AdjacentRoomIndex( idxRoom, 0 );
			slot = MAX_ADJACENT_ROOMS;
			while( curRoom != ADJACENT_ROOM_NULL )
			{
				DBSTREAM1( cstats << "backward" << std::endl; )
				AssertMsg( ( _roomSlotMap[curRoom] == ADJACENT_ROOM_NULL )
					|| ( _roomSlotMap[curRoom] == slot ),
					"_roomSlotMap[" << curRoom << "] == " << _roomSlotMap[curRoom] << std::endl );

				// mark
				_roomSlotMap[curRoom] = slot;
				DBSTREAM1( cstats << curRoom << "->" << slot << std::endl; )

				--slot;
				if( slot < 0 )
					slot = MAX_ADJACENT_ROOMS;

				int idxStartRoom = curRoom;
				for( int idxAdjRoom = 0; idxAdjRoom < MAX_ADJACENT_ROOMS; ++idxAdjRoom )
				{
					curRoom = AdjacentRoomIndex( idxStartRoom, idxAdjRoom );
					if( ( curRoom != ADJACENT_ROOM_NULL ) && ( _roomSlotMap[curRoom] == ADJACENT_ROOM_NULL ) )
					{
						break;
					}
					else
					{
						curRoom = ADJACENT_ROOM_NULL;
					}
				}
			}
		}
	}

  #if DO_ASSERTIONS
	for( int current = 0; current < _numRooms; ++current )
	{
		int previous = AdjacentRoomIndex( current, 0 );
		int next = AdjacentRoomIndex( current, 1 );

		int cur_idx = _roomSlotMap[current];
		assert( cur_idx >= 0 );
		assert( cur_idx <= MAX_ADJACENT_ROOMS );
		int prev_idx = ADJACENT_ROOM_NULL;
		int next_idx = ADJACENT_ROOM_NULL;

		if( previous != ADJACENT_ROOM_NULL )
		{
			prev_idx = _roomSlotMap[previous];

			// make sure all three slots are in range
			assert( prev_idx >= 0 );
			assert( prev_idx <= MAX_ADJACENT_ROOMS );

			// make sure all three slots are unique
			assert( prev_idx != cur_idx );
		}

		if( next != ADJACENT_ROOM_NULL )
		{
			next_idx = _roomSlotMap[next];

			// make sure all three slots are in range
			assert( next_idx >= 0 );
			assert( next_idx <= MAX_ADJACENT_ROOMS );

			// make sure all three slots are unique
			assert( next_idx != cur_idx );
		}

		if( previous != ADJACENT_ROOM_NULL && next != ADJACENT_ROOM_NULL )
		{
			// make sure all three slots are unique
			assert( prev_idx != next_idx );
		}

	  #if SW_DBSTREAM
		cstats << current << "->" << cur_idx;
		if( previous != ADJACENT_ROOM_NULL ) cstats << ", " << previous << "->" << prev_idx;
		if( next != ADJACENT_ROOM_NULL ) cstats << ", " << next << "->" << next_idx;
		cstats << std::endl;
	  #endif
	}
  #endif

  // kts added
#if SW_DBSTREAM
	croomslots << "room slot list:" << std::endl;
	for(int debugIndex=0;debugIndex<_numRooms;debugIndex++)
		croomslots << debugIndex << ": " << _roomSlotMap[debugIndex] << std::endl;
#endif

}

//=============================================================================

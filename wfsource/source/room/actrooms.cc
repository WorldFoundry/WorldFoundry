//==============================================================================
// actrooms.cc:
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

//==============================================================================
// Description:
// Original Author: Kevin T. Seghetti
//==============================================================================

#include <memory/memory.hp>
#include <gfx/vmem.hp>
#include <cpplib/libstrm.hp>
#include <asset/assslot.hp>
#include <physics/collision.hp>
#include "room.hp"
#include "rooms.hp"
#include "actrooms.hp"

//==============================================================================

extern collisionEvent collisionEventList[];	// declared in collisio.cc
extern int collisionEventListLength;
extern PhysicalObject* recollisionList[];
extern int recollisionListLength;
                      
//==============================================================================

void
ActiveRooms::Construct(int numRooms)
{
	_memory.Validate();
	assert(MAX_ACTIVE_ROOMS == VideoMemory::MAX_TRANSIENT_SLOTS);
	RangeCheckExclusive(0,numRooms,500);			// kts arbitrary
	_numRooms = numRooms;
	_tblFromRooms = new (_memory) int[_numRooms];
	ValidatePtr( _tblFromRooms );
	_tblToRooms = new (_memory) int[_numRooms];
	ValidatePtr( _tblToRooms );
	for(int index=0;index<MAX_ACTIVE_ROOMS;index++)
		_activeRooms[index] = NULL;
	Validate();
}

//==============================================================================

void
ActiveRooms::InitActiveRoom( int roomIndex, const LevelRooms& lRooms)
{
	DBSTREAM2( croom << "ActiveRooms::InitActiveRoom:" << std::endl; )

	assert( roomIndex >= 0 );
	assert( roomIndex < _numRooms );
	assert( lRooms.GetRoom(roomIndex).GetAdjacentRoom( 0 ) == &lRooms.GetRoom(roomIndex) );
	DBSTREAM1( croom << "ActiveRooms::initActiveRoom()" << std::endl; )

	for ( int activeRoomIndex = 0; activeRoomIndex < MAX_ACTIVE_ROOMS; ++activeRoomIndex )
	{
		_activeRooms[activeRoomIndex] = lRooms.GetRoom(roomIndex).GetAdjacentRoom( activeRoomIndex );
		if( _activeRooms[activeRoomIndex] )
		{
			int roomIndex = _activeRooms[activeRoomIndex]->GetRoomIndex();
			assert(roomIndex >= 0);
			AssertMsg(roomIndex < lRooms.NumberOfRooms(),"roomIndex = " << roomIndex << ", number of rooms = " << lRooms.NumberOfRooms());
			DBSTREAM2 (croomslots << "roomIndex = " << roomIndex << std::endl; )
			DBSTREAM2( croomslots << "calling LoadRoomSlot:" << roomIndex << "," << lRooms.GetSlotIndex(roomIndex) << std::endl; )
			_assetManager.LoadRoomSlot(roomIndex,lRooms.GetSlotIndex(roomIndex));
			BaseObjectIteratorWrapper rendIter = _activeRooms[activeRoomIndex]->ListIter(ROOM_OBJECT_LIST_RENDER);
			while(!rendIter.Empty())
			{
            rendIter.Validate();
            PhysicalObject* po = dynamic_cast<PhysicalObject*>(&(*rendIter));
            assert(ValidPtr(po));
				int assetSlot = lRooms.GetSlotIndex(roomIndex);

				if(po->GetMovementBlockPtr()->MovesBetweenRooms )
					assetSlot = VideoMemory::PERMANENT_SLOT;

				DBSTREAM2 (croomslots << "calling BindAssets on object:" << *rendIter << " in slot #" << roomIndex << std::endl; )
		   	po->BindAssets(_assetManager.GetAssetSlot(assetSlot).GetSlotMemory());
				++rendIter;
			}
		_activeRooms[ activeRoomIndex ]->BindAssets();
		}
	}
	Validate();
}

//==============================================================================

void
ActiveRooms::ChangeActiveRoom( int toRoom )
{
	Validate();
	DBSTREAM2 (croomslots << "ActiveRooms::ChangeActiveRoom:" << std::endl; )
#if SW_DBSTREAM
	int fromRoom = _activeRooms[0]->GetRoomIndex();
	assert( fromRoom != toRoom );
#endif

	DBSTREAM2( croomslots << "fromRoom " << fromRoom << " toRoom " << toRoom << std::endl; )

#if SW_DBSTREAM
	croomslots << "active room array before:" << std::endl;
	for( int debugactiveRoomIndex = 0; debugactiveRoomIndex<MAX_ACTIVE_ROOMS; debugactiveRoomIndex++)
		croomslots << debugactiveRoomIndex << ": " << _activeRooms[debugactiveRoomIndex] << std::endl;
#endif

    {		                            // until msvc gets with the 90's
	for(int activeRoomIndex=0;activeRoomIndex<MAX_ACTIVE_ROOMS;activeRoomIndex++)
		_fromActiveRooms[activeRoomIndex] = _activeRooms[activeRoomIndex];
	}

	// clear the from and to room arrays
	for( int roomIndex = 0; roomIndex < _numRooms; ++roomIndex )
		_tblFromRooms[roomIndex] = _tblToRooms[roomIndex] = 0;

	// create the from and to room arrays, and update the _activeRooms array
	for( int activeRoomIndex = 0; activeRoomIndex < MAX_ACTIVE_ROOMS; ++activeRoomIndex )
	{
		if ( _activeRooms[activeRoomIndex] )				// Mark from room as having a texture map
		{
			assert(_activeRooms[activeRoomIndex]->GetRoomIndex() >= 0);
			assert(_activeRooms[activeRoomIndex]->GetRoomIndex() < _levelRooms.NumberOfRooms());
			_tblFromRooms[ _activeRooms[activeRoomIndex]->GetRoomIndex() ] = 1;
		}

		Room* pActiveRoom = _levelRooms.GetRoom(toRoom).GetAdjacentRoom( activeRoomIndex );
		_activeRooms[activeRoomIndex] = pActiveRoom;		// Update _activeRooms array to contain the rooms we are switching to

		if( pActiveRoom )									// Mark to room as having a texture map
			_tblToRooms[ pActiveRoom->GetRoomIndex() ] = 1;
	}

#if SW_DBSTREAM
	{
	croomslots << "from active rooms, active room array after:" << std::endl;
	for( int debugactiveRoomIndex = 0; debugactiveRoomIndex<MAX_ACTIVE_ROOMS; debugactiveRoomIndex++)
		croomslots << debugactiveRoomIndex << ": " << _fromActiveRooms[debugactiveRoomIndex] << "," << _activeRooms[debugactiveRoomIndex] << std::endl;
	croomslots << "from and to arrays: " << std::endl;
	for( int debugroomIndex = 0; debugroomIndex < _numRooms; ++debugroomIndex )
	{
		croomslots << debugroomIndex << '\t' << _tblFromRooms[debugroomIndex] << "\t\t" <<  _tblToRooms[debugroomIndex] << std::endl;
	}
	}
#endif

	// unload old room assets
	for( int oldRoomIndex = 0; oldRoomIndex < _numRooms; ++oldRoomIndex )
	{
		if( _tblFromRooms[oldRoomIndex] && _tblToRooms[oldRoomIndex] == 0 )
		{
			// Indicate that room is not loaded
			// update the from and to room arrays
			for( int idxActiveRoom = 0; idxActiveRoom < MAX_ACTIVE_ROOMS; ++idxActiveRoom )
			{
				if (_fromActiveRooms[ idxActiveRoom ] != NULL)
				{
					if ( _fromActiveRooms[ idxActiveRoom ]->GetRoomIndex() == oldRoomIndex )
					{
						DBSTREAM2( croomslots << "Freeing room #" << oldRoomIndex << std::endl; )
						BaseObjectIteratorWrapper rendIter = _fromActiveRooms[idxActiveRoom]->ListIter(ROOM_OBJECT_LIST_RENDER);
						DBSTREAM2( croomslots << "it thinks its room # is " << *_fromActiveRooms[idxActiveRoom] << std::endl; )

						while(!rendIter.Empty())
						{
		   				(*rendIter).UnBindAssets();
							++rendIter;
						}
						_fromActiveRooms[ idxActiveRoom ] = NULL;

						_activeRooms[ idxActiveRoom ]->UnBindAssets();
						break;
					}
				}
			}
			_assetManager.FreeRoomSlot(_levelRooms.GetSlotIndex(oldRoomIndex));
		}
	}

	// load new rooms
	for( int newRoomIndex = 0; newRoomIndex < _numRooms; ++newRoomIndex )
	{
		if( _tblFromRooms[ newRoomIndex ] == 0 && _tblToRooms[ newRoomIndex ] )
		{	// loaded room i:
			// Mark room as not having a texture map
//kts			_tblToRooms[ newRoomIndex ] = 0;

			int idxActiveRoom;
			for( idxActiveRoom = 0; idxActiveRoom < MAX_ACTIVE_ROOMS; ++idxActiveRoom )
			{
				if (_activeRooms[ idxActiveRoom ] != NULL)
				{
					if ( _activeRooms[ idxActiveRoom ]->GetRoomIndex() == newRoomIndex )
					{
//						_activeRooms[ idxActiveRoom ] = NULL;
						assert(newRoomIndex >= 0);
						AssertMsg(newRoomIndex < _levelRooms.NumberOfRooms(),"roomIndex = " << newRoomIndex << ", number of rooms = " << _levelRooms.NumberOfRooms());
						DBSTREAM2( croomslots << "calling loadRoomSlot:" << newRoomIndex << "," << _levelRooms.GetSlotIndex(newRoomIndex) << std::endl; )
						_assetManager.LoadRoomSlot(newRoomIndex,_levelRooms.GetSlotIndex(newRoomIndex));
						BaseObjectIteratorWrapper rendIter = _activeRooms[idxActiveRoom]->ListIter(ROOM_OBJECT_LIST_RENDER);
						while(!rendIter.Empty())
						{
		   				(*rendIter).BindAssets(_assetManager.GetAssetSlot(_levelRooms.GetSlotIndex(newRoomIndex)).GetSlotMemory());
							++rendIter;
						}
						_activeRooms[ idxActiveRoom ]->BindAssets();
						break;
					}
				}
			}
			assert(idxActiveRoom < MAX_ACTIVE_ROOMS);
		}
	}
#if SW_DBSTREAM
	{
	croomslots << "from active rooms, active room array way after:" << std::endl;
	for( int debugactiveRoomIndex = 0; debugactiveRoomIndex<MAX_ACTIVE_ROOMS; debugactiveRoomIndex++)
		croomslots << debugactiveRoomIndex << ": " << _fromActiveRooms[debugactiveRoomIndex] << "," << _activeRooms[debugactiveRoomIndex] << std::endl;
	croomslots << "from and to arrays: " << std::endl;
	for( int debugroomIndex = 0; debugroomIndex < _numRooms; ++debugroomIndex )
	{
		croomslots << debugroomIndex << '\t' << _tblFromRooms[debugroomIndex] << "\t\t" <<  _tblToRooms[debugroomIndex] << std::endl;
	}
	}
#endif
	DBSTREAM1( std::cout << _assetManager << std::endl; )
	DBSTREAM2( croomslots << "change active room finished " << std::endl; )
	Validate();
}

//==============================================================================

void
ActiveRooms::FreeActiveRooms()
{
	assert(0);
}

//=============================================================================
// updates the active room array to reflect the current state of active rooms
//

void
ActiveRooms::AttachActiveRoom( int idxRoom, int idxToRoom )
{
	Validate();
	int adjRoom;

	// step through all of the rooms
	for(adjRoom = 0; adjRoom < MAX_ACTIVE_ROOMS; ++adjRoom )
	{
		// see if room is adjacent to the room of interest
		Room *ptrTempRoom =
			_levelRooms.GetRoom(idxToRoom).GetAdjacentRoom( adjRoom );

		// if pointer is non-zero, it's an adjacent room
		if ( ptrTempRoom != NULL )
		{
			assert( ValidPtr( ptrTempRoom ) );
			// if this is the room of interest
			if (ptrTempRoom->GetRoomIndex() == idxRoom)
			{
				// connect this to room of interest
				_activeRooms[ adjRoom ] = ptrTempRoom;

				// Mark room as having a texture map
				_tblToRooms[ idxRoom ] = 1;
			}
		}
	}
	Validate();
}

//==============================================================================
// decides which room is the active one, and makes it active

void
ActiveRooms::UpdateRoom(const PhysicalObject* watchObject)
{
   assert(ValidPtr(watchObject));
	Validate();
	assert( ValidPtr( _activeRooms ));
	DBSTREAM2( croom << "ActiveRooms::UpdateRoom:" << std::endl; )
	assert( ValidPtr( _activeRooms[0] ) );									// insure current room is set
	if( !_activeRooms[0]->CheckCollision( * watchObject ))
	{ // figure out new room that I'm in now
		for ( int roomIndex = 0; roomIndex < _numRooms; roomIndex++ )
		{
			assert(ValidPtr(&_levelRooms.GetRoom(roomIndex)));
			if ( _levelRooms.GetRoom(roomIndex).CheckCollision( * watchObject ))
			{
				ChangeActiveRoom( roomIndex );
				break;
			}
		}
	}
	assert( ValidPtr( _activeRooms[0] ));
	Validate();
}

//==============================================================================
// This function returns false if no rooms are pending, or true if there are
// if blocking is true, it will wait until one room is loaded and then
// return a value telling if there are still any more rooms pending.
//
bool
ActiveRooms::WaitRoomLoad( bool /*blocking*/ )
{
	Validate();
	return false;
}

//=============================================================================
// kts there should be a function which returns an active rooms iterator 
// (activerooms should just be a list of pointers to rooms)


void
ActiveRooms::DetectCollision(const Clock& currentTime)
{
//	DBSTREAM2( croom << "Level::detectCollision" << std::endl; )
	// Make sure all collision events were processed last frame
	Validate();
#if DO_ASSERTIONS
	if (collisionEventListLength > 0)
	{
		DBSTREAM1(
		std::cerr << "There were unresolved collision event(s) left over from last frame:" << std::endl;
		while (collisionEventListLength > 0)
		{
			std::cerr << *collisionEventList[collisionEventListLength-1].object1 << " vs." << std::endl;
			std::cerr << *collisionEventList[collisionEventListLength-1].object2 << std::endl;
			std::cerr << "At time = " << collisionEventList[(collisionEventListLength--) - 1].eventTime << std::endl << std::endl;
		}
		)
		assert(0);
	}
	if (recollisionListLength > 0)
	{
		DBSTREAM1(
		std::cerr << "There are re-collisions left over from last frame:" << std::endl;
		while (recollisionListLength > 0)
		{
			std::cerr << *recollisionList[(recollisionListLength--) - 1] << std::endl;
		}
		)
		assert(0);
	}
#endif

 	const Room* room0 = GetActiveRoom(0);
	{
   	assert( ValidPtr( room0 ) );
   	DBSTREAM2( croom << "QL::dC: checking room0 " <<room0->GetRoomIndex() << "<->" << room0->GetRoomIndex() << std::endl; )
   	CheckSameRoom( room0->ListIter(ROOM_OBJECT_LIST_COLLIDE), *room0,  currentTime, ROOM_OBJECT_LIST_COLLIDE );
	}

	const Room* room1 = GetActiveRoom(1);
	if( room1 )
	{
		{
			DBSTREAM2( croom << "QL::dC: checking room " <<room1->GetRoomIndex() << "<->" << room1->GetRoomIndex() << std::endl; )
			CheckSameRoom( room1->ListIter(ROOM_OBJECT_LIST_COLLIDE), * room1,  currentTime, ROOM_OBJECT_LIST_COLLIDE );
		}
		{
			DBSTREAM2( croom << "QL::dC: checking objects in room " <<room0->GetRoomIndex() << " against objects in room " << room1->GetRoomIndex() << std::endl; )
			CheckOverlappedRoom( room0->ListIter(ROOM_OBJECT_LIST_COLLIDE), * room1,  currentTime,ROOM_OBJECT_LIST_COLLIDE );
		}
		{
			DBSTREAM2( croom << "QL::dC: checking objects in room " <<room1->GetRoomIndex() << " against objects in room " << room0->GetRoomIndex() << std::endl; )
			CheckOverlappedRoom(room1->ListIter(ROOM_OBJECT_LIST_COLLIDE), * room0,  currentTime, ROOM_OBJECT_LIST_COLLIDE );
		}

	}
	const Room* room2 = GetActiveRoom(2);
	if( room2 )
	{
		{
			DBSTREAM2( croom << "QL::dC: checking room " <<room2->GetRoomIndex() << "<->" << room2->GetRoomIndex() << std::endl; )
			CheckSameRoom( room2->ListIter(ROOM_OBJECT_LIST_COLLIDE), * room2,  currentTime ,ROOM_OBJECT_LIST_COLLIDE );
		}
		{
			DBSTREAM2( croom << "QL::dC: checking objects in room " <<room2->GetRoomIndex() << " against objects in room " << room0->GetRoomIndex() << std::endl; )
			CheckOverlappedRoom( room2->ListIter(ROOM_OBJECT_LIST_COLLIDE), * room0,  currentTime, ROOM_OBJECT_LIST_COLLIDE );
		}
		{
			DBSTREAM2( croom << "QL::dC: checking objects in room " <<room0->GetRoomIndex() << " against objects in room " << room2->GetRoomIndex() << std::endl; )
			CheckOverlappedRoom( room0->ListIter(ROOM_OBJECT_LIST_COLLIDE), * room2,  currentTime, ROOM_OBJECT_LIST_COLLIDE );
		}
	}

	int recollisionCount = 0;

	while (collisionEventListLength > 0)
	{
		// Sort collision list by time of event
		SortCollisionEventList(currentTime);

		// Resolve all collisions detected
		while (collisionEventListLength > 0)
		{
			// Process the collision at the END of the list, and decrement the list length
			ResolveCollisionEvent( collisionEventList[collisionEventListLength-1], currentTime );
			collisionEventListLength--;
		}

		// re-collide any objects who changed their position as a result of collision
		while (recollisionListLength > 0)
		{
			if (recollisionCount++ > 1000)
			{
				DBSTREAM3( ccollision << "**** RECOLLISION COUNT EXCEEDED.  SHIT. ****" << std::endl; )
				AssertMsg(0, "I've recollided way too much this frame...");
			}
			DBSTREAM3( ccollision << "Re-collision depth = " << recollisionCount << std::endl; )

			room0->CheckCollisionWithObjects( *recollisionList[(recollisionListLength) - 1], currentTime,ROOM_OBJECT_LIST_COLLIDE );
			if (room1)
				room1->CheckCollisionWithObjects( *recollisionList[(recollisionListLength) - 1], currentTime, ROOM_OBJECT_LIST_COLLIDE );
			if (room2)
				room2->CheckCollisionWithObjects( *recollisionList[(recollisionListLength) - 1], currentTime, ROOM_OBJECT_LIST_COLLIDE );
			recollisionListLength--;
		}
	}
	Validate();
}

//==============================================================================


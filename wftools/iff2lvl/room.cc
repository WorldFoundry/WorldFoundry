//============================================================================
// room.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level room data
	History:
			Created 05-05-95 10:26am Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================

#include "global.hp"
#include <stdio.h>
//#include <stl/bstring.h>

#include "hdump.hp"

//#include "pigtool.h"
#include "level.hp"
#include "room.hp"
#include <oas/levelcon.h>		// included from velocity\source


//============================================================================

QRoom::QRoom()
{
}

//============================================================================

void
QRoom::SetBox(const QColBox& newBox)			// reference to box object which defines size and position of this room
{
	box = newBox;
}

//============================================================================

QRoom::~QRoom()
{
	objects.erase(objects.begin(), objects.end());
}

//============================================================================

bool
QRoom::InsideCheck(QObject* object) const			// returns TRUE if object coordinates are inside of room coordinates
{
// kts compute center of object for purposes of determining if one object is in another (rooms)


	Point3 point(
		(object->GetColBox().GetMax().x + object->GetColBox().GetMin().x)/2,
		(object->GetColBox().GetMax().y + object->GetColBox().GetMin().y)/2,
		(object->GetColBox().GetMax().z + object->GetColBox().GetMin().z)/2
	);

	point += object->Position();

	DBSTREAM3( cdebug << "QRoom::InsideCheck: Object centroid (global space) is: " << point << endl; )

	return(box.InsideCheck(point));
//	return(box.InsideCheck(object->Position()));
}

//============================================================================

void
QRoom::Add(QObject* object)
{
	objects.push_back(PtrObj<QObject> (object));
}

//============================================================================

size_t
QRoom::SizeOfOnDisk(void) const
{
	size_t size;
	size = (sizeof(_RoomOnDisk)+objects.size()*sizeof(_RoomOnDiskEntry));
	if(size & 3)
		size += (4-(size&3));
	return(size);
}

//============================================================================

char* adjacentRoomNameList[] =
{
	"Adjacent Room 1",
	"Adjacent Room 2",
};


void
QRoom::Save(ostream& lvl,const QLevel& level)
{
	DBSTREAM3( cprogress << "QRoom::Save" << endl; )
	DBSTREAM3( cdebug << "QRoom::Save: contains <" << objects.size() << "> objects" << endl; )

	_RoomOnDisk* diskRoom = (_RoomOnDisk*)new char[SizeOfOnDisk()];
	assert(diskRoom);
	_RoomOnDiskEntry* diskRoomEntry = (_RoomOnDiskEntry*)((char*)diskRoom + sizeof(_RoomOnDisk));

	assert(objects.size() < MAX_OBJECTS_PER_ROOMS);
	diskRoom->count = objects.size();
	diskRoom->roomObjectIndex = level.FindObjectIndex(_name);
	box.Write(&diskRoom->boundingBox);

	// read adjacent room references from this room's .oad
	assert(MAX_ADJACENT_ROOMS == 2);					// change adjacentRoomNameList if this isn't 2
	for(int adjRoomIndex=0;adjRoomIndex < MAX_ADJACENT_ROOMS; adjRoomIndex++)
	 {
		DBSTREAM3( cdebug << "  QRoom::Save: top of loop" << endl; )
		diskRoom->adjacentRooms[adjRoomIndex] = ADJACENT_ROOM_NULL;		// if not found, will default to NULL
		const QObjectAttributeDataEntry* oadEntry;
		oadEntry = oad.GetEntryByName(adjacentRoomNameList[adjRoomIndex]);
		if(oadEntry)
		 {
			int roomIndex;
			if(oadEntry->GetString().length())
			 {
				DBSTREAM3( cdebug << "  QRoom::Save: looking up room object named <" << oadEntry->GetString() << ">" << endl; )
//				roomIndex = level.FindObjectIndex(oadEntry->string);
				roomIndex = FindRoomIndex(oadEntry->GetString(),level);
				if(roomIndex >= 0)
					diskRoom->adjacentRooms[adjRoomIndex] = roomIndex;
				else
					cerror << "LevelCon Error: room named <" << oadEntry->GetString() << "> referenced from room named <" << _name << "> not found" << endl;
			 }
			DBSTREAM3( cdebug << "QRoom::Save: adjacent room index <" << adjRoomIndex << ">" << endl; )
//			DBSTREAM3( cdebug << "QRoom::Save: adjacent room index <" << adjRoomIndex << "> set to <" << diskRoom->adjacentRooms[adjRoomIndex] << ">" << endl; )
		 }
		else
		 {
			cerror << "LevelCon Error: Can't find oad field <" << adjacentRoomNameList[adjRoomIndex] << ">" << endl;
			diskRoom->adjacentRooms[adjRoomIndex] = ADJACENT_ROOM_NULL;
		 }
	 }

	// print list of objects in this room
	DBSTREAM3( cdebug << "room save done" << endl; )
	for(unsigned int index=0;index < objects.size(); index++)
	 {
		DBSTREAM5( cdebug << "QRoom::Save: contains object address <" << objects[index] << "> named <" << objects[index]->GetName() << ">"; )
		diskRoomEntry[index].object = level.GetObjectIndex(objects[index]);
		DBSTREAM5( cdebug << " with offset of <" << diskRoomEntry[index].object << ">" << endl; )
	 }

	DBSTREAM3( cdebug << "room save really done" << endl; )
//	fwrite(diskRoom,SizeOfOnDisk(),1,fp);
	lvl.write( (const char*)diskRoom, SizeOfOnDisk() );
}

//============================================================================

int
QRoom::FindRoomIndex(const string& name,const QLevel& level) const
{
#if 1
	for(int idxRoom = 0; idxRoom < level.GetRoomCount(); idxRoom++)
	{
		if(level.GetRoom(idxRoom).GetName() == name)
				return(idxRoom);
	}
#else
	cdebug << "FindRoomIndex: begin, object count = " << level.GetObjectCount() << endl;
	int roomIndex=0;
	int oadIndex;
	for(int index=0;index<level.GetObjectCount();index++)
	{
//		assert(level.objectOADs.length() > oadIndex);

		if ( level.GetObject(index).GetName() == name )
		{
			oadIndex = level.GetObject(index).GetTypeIndex();
			if(level.GetOAD(oadIndex).GetEntryByType(LEVELCONFLAG_ROOM))
			{
				cdebug << "FindRoomIndex: found" << endl;
				return(roomIndex);
			}
			else
			{
				cdebug << "FindRoomIndex: failed, not room" << endl;
				return(-1);					// found, but not room object
			}
		}
		if(level.GetOAD(oadIndex).GetEntryByType(LEVELCONFLAG_ROOM))
			roomIndex++;
	}
#endif
	return(-1);
}

//============================================================================
// Printing

ostream& operator<<(ostream& s, const QRoom &room)
{
	s << "QRoom Dump" << endl;
	for(unsigned int index=0;index < room.objects.size(); index++)
		s << *room.objects[index];
	return(s);
}

//============================================================================

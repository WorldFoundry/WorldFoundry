//============================================================================
// Room.hp:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			list of objects in a particular area, called a room
	History:
			Created	08-07-95 11:21am Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================
// use only once insurance
#ifndef _ROOM_HP
#define _ROOM_HP

//============================================================================

#include "global.hpp"

#include <iostream.h>

#include <stl/bstring.h>

//#include "pigtool.h"
#include "colbox.hpp"
#include "object.hpp"
#include "ptrobj.hpp"

class QLevel;

// kts arbitrary
#define MAX_ROOMS 200
#define MAX_OBJECTS_PER_ROOMS 300

//============================================================================
// in memory representation of a list of objects

class QRoom
{
public:
	QRoom();							// this is so the rouge stuff will work
	~QRoom();

	// accessors
	size_t SizeOfOnDisk(void) const;			// returns size in bytes of on-disk representation
	bool InsideCheck(QObject* object) const;			// returns TRUE if object coordinates are inside of room coordinates
	operator==(const QRoom& left) const { return(0); }			// kts cannot compare rooms
	long length(void) const { return(objects.size()); }
	const QObject& GetObjectByIndex(unsigned int index) const { assert(index < objects.size());  return objects[index]; }

	const string& GetName() const { return( _name); }
	const QColBox& GetColBox() const { return(box); }

	// modifiers
	void SetBox(const QColBox& newBox);
	void SetOAD(const QObjectAttributeData& newOad) { oad = newOad; }
	void SetName(const string& newName) { _name = newName; }
	const string& GetName() { return _name; }
	void Add(QObject* object);
	void Save(ostream& lvl, const QLevel& level);

	// Printing
	friend ostream& operator<<(ostream& s, const QRoom &o);
private:
	int FindRoomIndex(const string& name, const QLevel& level) const;

	string _name;
	QColBox box;
	vector< PtrObj<QObject> > objects;				// array of pointers to objects
	QObjectAttributeData oad;
};

//============================================================================
#endif
//============================================================================

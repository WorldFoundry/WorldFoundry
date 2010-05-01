//============================================================================
// object.hp:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level objects
	History:
			Created	05-04-95 11:13am Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================
// use only once insurance
#ifndef _oBJECT_HP
#define _oBJECT_HP

//============================================================================

#include "global.hpp"
#include "colbox.hpp"
#include "oad.hpp"

class QLevel;

//============================================================================
// in memory representation of a 3D object

class QObject
{
public:
	enum { NAMELEN=32 };
	QObject(const char* name,int16 type, const Point3& newPosition,
			const Point3& newScale, const Euler& newRotation,
			const QColBox& collision, int32 newOadFlags,
			int16 pathIndex, const QObjectAttributeData& newOad);
	~QObject();
	void Save(ostream& lvl, const QLevel& level);
	size_t SizeOfOnDisk(void) const;			// returns size in bytes of on-disk representation

 	int16 GetTypeIndex(void) const { return(type); }
	const char* GetName(void) const { return(name); }

	const Point3& Position(void) const { return(position); }
	const Point3& Scale(void) const { return(scale); }
	const Euler& Rotation(void) const { return(rotation); }
	const QColBox& GetColBox(void) const { return(collision); }
	const QObjectAttributeData& GetOAD(void) const { return(oad); }
	int16 GetPathIndex(void) const { return pathIndex; }

	// Printing
	friend ostream& operator<<(ostream& s, const QObject &o);

	operator==(const QObject& left) const
	 {
		return( !strcmp(name,left.name) &&
				type == left.type &&
				position == left.position &&
				scale == left.scale &&
				rotation.a == left.rotation.a &&
				rotation.b == left.rotation.b &&
				rotation.c == left.rotation.c &&
				collision == left.collision &&
				oadFlags == left.oadFlags &&
				pathIndex == left.pathIndex &&
				oad == left.oad
			  );
	 }

	 operator< (const QObject& left) const		// Dummy operator to make STL happy
	 {
		 return false;
	 }

private:
	char name[NAMELEN];
	int16 type;
	Point3 position;
	Point3 scale;
	Euler rotation;
	QColBox collision;				// collision box
	int32 oadFlags;
	int16 pathIndex;

	QObjectAttributeData oad;
//	char* typeData;
//	size_t typeDataSize;

};

//============================================================================
#endif
//============================================================================

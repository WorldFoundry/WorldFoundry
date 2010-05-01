//============================================================================
// path.hp:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of an object path
	History:
			Created	06-14-95 05:05pm Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================
// use only once insurance

#ifndef _path_HP
#define _path_HP

//============================================================================

#include "global.hpp"

#include <stl/vector.h>
#include "point6.hpp"

//============================================================================
// in memory representation of a 3D path

class QPath
{
public:
	QPath();
	QPath(const Point3& basePos, const Quat& baseRot);				// relative path
	~QPath();
	void Save(ostream& lvl);
	size_t SizeOfOnDisk(void);			// returns size in bytes of on-disk representation

//	void Add(const Point6& newPoint, TimeValue time);
	void AddPositionKey(TimeValue time, Point3& position);
	void AddRotationKey(TimeValue time, Quat& rotation);
//	int32 GetTimeIndex(TimeValue time) const;
//	void SetPoint(const Point6& newPoint, int32 index);
	Point3 GetPosition(TimeValue time) const;
	Quat GetRotation(TimeValue time) const;
	void AddPositionOffset(const Point3& offset);

//	const int Size(void) const;		// Returns number of keyframes in this path
	// Printing
	friend ostream& operator<<(ostream& s, const QPath &o);
	operator==(const QPath& left) const { return(0); }
private:
	Point3 basePosition;				// base XYZ for (relative) path
	Quat baseRotation;
	int16 positionXChannel;				// index into Level's channels vector
	int16 positionYChannel;
	int16 positionZChannel;
	int16 rotationAChannel;
	int16 rotationBChannel;
	int16 rotationCChannel;
};

//============================================================================
#endif
//============================================================================

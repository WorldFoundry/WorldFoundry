//============================================================================
// path.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of level path data
	History:
			Created 06-14-95 05:42pm Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <pclib/stdstrm.hp>
#include <pclib/hdump.hp>
#include <stl/algo.h>

#include "pigtool.h"
#include "path.hp"
#include <source\levelcon.h>		// included from velocity\source

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================

QPath::QPath()
{
	base.x = 0;
	base.y = 0;
	base.z = 0;
}

//============================================================================
// relative path
QPath::QPath(br_scalar x, br_scalar y, br_scalar z)
{
	base.x = x;
	base.y = y;
	base.z = z;
}

//============================================================================

QPath::~QPath()
{
}

//============================================================================

void
QPath::Add(const QPoint& newPoint,int32 time)
{
	assert(keyFramePoints.size() == keyFrameTimes.size());
	keyFramePoints.push_back(newPoint);
	keyFrameTimes.push_back(time);
	assert(keyFramePoints.size() == keyFrameTimes.size());
}

//============================================================================

int32
QPath::GetTimeIndex(int32 time) const
{
	vector<int32>::const_iterator where;
	where = find(keyFrameTimes.begin(),keyFrameTimes.end(),time);

	int32 index = where - keyFrameTimes.begin();

	assert(index < Size());
	return index;
}

//============================================================================

void
QPath::SetPoint(const QPoint& newPoint,int32 index)
{
	assert(index < Size());
	keyFramePoints[index] = newPoint;
	assert(keyFramePoints.size() == keyFrameTimes.size());
}

//============================================================================

const QPoint&
QPath::GetPoint(int32 index) const
{
	assert(index < Size());
	return keyFramePoints[index];
}

//============================================================================

const int
QPath::Size(void) const		// Returns number of keyframes in this path
{
	AssertMsg(keyFramePoints.size() == keyFrameTimes.size(), "Possible cause: object with hierarchical path another object doesn't have a path" );
	return (keyFramePoints.size());
}

//============================================================================
size_t
QPath::SizeOfOnDisk(void)
{
	assert(keyFramePoints.size());				// cannot have zero frames
	assert(keyFramePoints.size() == keyFrameTimes.size());
	return(sizeof(_PathOnDisk) + ((keyFramePoints.size()) * sizeof(_PathOnDiskEntry)) );
}

//============================================================================

QPath::Save(FILE* fp)
{
	assert(keyFramePoints.size() == keyFrameTimes.size());
	DBSTREAM3( cdebug << "QPath::Save" << endl; )
	DBSTREAM3( cdebug << "QPath::Save: contains <" << keyFramePoints.size() << "> paths" << endl; )

	_PathOnDisk* diskPath = (_PathOnDisk*)new char[SizeOfOnDisk()];
	assert(diskPath);
	_PathOnDiskEntry* diskPathEntry = (_PathOnDiskEntry*)((char*)diskPath + sizeof(_PathOnDisk));

	diskPath->count = keyFramePoints.size();
	diskPath->base.x = base.x;
	diskPath->base.y = base.y;
	diskPath->base.z = base.z;
	for(int index=0;index < keyFramePoints.size(); index++)
	 {
		diskPathEntry[index].x = keyFramePoints[index].x() - base.x;
		diskPathEntry[index].y = keyFramePoints[index].y() - base.y;
		diskPathEntry[index].z = keyFramePoints[index].z() - base.z;
		diskPathEntry[index].frame = keyFrameTimes[index];
	 }
	fwrite(diskPath,SizeOfOnDisk(),1,fp);
}

//============================================================================
// Printing

ostream& operator<<(ostream& s, const QPath &path)
{
	s << "QPath Dump" << endl;
//	s << "X = " << path._x << "  Y = " << path._y << "  Z = " << path._z << endl;
	return s;
}

//============================================================================

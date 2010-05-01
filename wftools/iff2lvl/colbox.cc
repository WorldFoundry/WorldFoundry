//============================================================================
// ColBox.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of object collision box
	History:
			Created 07-31-95 02:10pm Kevin T. Seghetti

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

#include "hdump.hp"

//#include "pigtool.h"
#include "level.hp"
#include "colbox.hp"
#include <oas/levelcon.h>		// included from wfsource

//extern Interface* gMaxInterface;	// Global pointer to MAX interface class

//============================================================================

QColBox::QColBox()
{
	_min = Point3(0,0,0);
	_max = Point3(0,0,0);
}

//============================================================================

QColBox::QColBox(Point3 min, Point3 max)
{
	_min = min;
	_max = max;

    AssertMsg(_min.x <= _max.x,	" min = " << min << ", max = " << max);
    AssertMsg(_min.y <= _max.y,	" min = " << min << ", max = " << max);
    AssertMsg(_min.z <= _max.z,	" min = " << min << ", max = " << max);
}

//============================================================================

QColBox::~QColBox()
{
}

//============================================================================

QColBox&
QColBox::operator+=(const Point3& offset)							// add offset to all points in colbox
{
	_min += offset;
	_max += offset;
	return(*this);
}

//============================================================================

bool
QColBox::InsideCheck(const Point3 point) const
{
	DBSTREAM3( cdebug << "Checking against colbox:" << endl << *this << endl; )

	// kts changed to not include edges (removed =)
	// kts changed to include edges (added =)
	if(
		point.x >= _min.x && point.x <= _max.x &&
		point.y >= _min.y && point.y <= _max.y &&
		point.z >= _min.z && point.z <= _max.z
	  )
	 {
		DBSTREAM3( cdebug << "   Returning TRUE" << endl; )
		return(true);
	 }
	DBSTREAM3( cdebug << "   Returning FALSE" << endl; )
	return(false);
}

//============================================================================

inline float
colboxabs(float a)
{
	return(a<0?-a:a);
}

double
QColBox::GetVolume() const
{
	double xSize,ySize,zSize;

	xSize = colboxabs(_min.x - _max.x);
	ySize = colboxabs(_min.y - _max.y);
	zSize = colboxabs(_min.z - _max.z);
	assert(xSize >= 0);
	assert(ySize >= 0);
	assert(zSize >= 0);
	double result = xSize * ySize * zSize;
//	cdebug << "QColBox::GetVolume: xSize = <" << xSize << ">, ySize = <" << ySize << ">, zSize = <" << zSize << ">, volume = <" << result << ">" << endl;
	return(result);
}

//============================================================================

size_t
QColBox::SizeOfOnDisk() const
{
	return(sizeof(_CollisionRectOnDisk));
}

//============================================================================
// Write out to the on disk structure

void
QColBox::Write(_CollisionRectOnDisk* destColBox) const
{
	destColBox->minX = WF_FLOAT_TO_SCALAR(_min.x);
	destColBox->minY = WF_FLOAT_TO_SCALAR(_min.y);
	destColBox->minZ = WF_FLOAT_TO_SCALAR(_min.z);
	destColBox->maxX = WF_FLOAT_TO_SCALAR(_max.x);
	destColBox->maxY = WF_FLOAT_TO_SCALAR(_max.y);
	destColBox->maxZ = WF_FLOAT_TO_SCALAR(_max.z);

	DBSTREAM3( cdebug << "QColBox::Write: writting colbox " << endl; )
	DBSTREAM3( cdebug << "  MinX: <" << WF_SCALAR_TO_FLOAT(destColBox->minX) << ">, MinY: <" << WF_SCALAR_TO_FLOAT(destColBox->minY) << "> MinZ: <" << WF_SCALAR_TO_FLOAT(destColBox->minZ) << ">" << endl; )
 	DBSTREAM3( cdebug << "  MaxX: <" << WF_SCALAR_TO_FLOAT(destColBox->maxX) << ">, MaxY: <" << WF_SCALAR_TO_FLOAT(destColBox->maxY) << "> MaxZ: <" << WF_SCALAR_TO_FLOAT(destColBox->maxZ) << ">" << endl; )
}

//============================================================================

void
QColBox::Bound(const Mesh& mesh, Matrix3& rotMatrix, Point3& objOffsetPos)
{
	AssertMessageBox(mesh.numVerts > 0, "mesh doesn't have any vertices" );

	DBSTREAM3( cdebug << "QColBox::Bound numVerts = " << mesh.numVerts << endl);

#pragma message ("KTS " __FILE__ ": finish this")
	assert(0);
#if 0
	Point3 tempPoint = mesh.verts[0];
	tempPoint = tempPoint * rotMatrix;
	tempPoint = tempPoint + objOffsetPos;
	float minx = tempPoint.x;
	float miny = tempPoint.y;
	float minz = tempPoint.z;
	float maxx = tempPoint.x;
	float maxy = tempPoint.y;
	float maxz = tempPoint.z;

	for(int index=0; index < mesh.numVerts; index++)
	{
		tempPoint = mesh.verts[index];
		tempPoint = tempPoint * rotMatrix;
		tempPoint = tempPoint + objOffsetPos;
		if (tempPoint.x < minx)
			minx = tempPoint.x;
		if (tempPoint.y < miny)
			miny = tempPoint.y;
		if (tempPoint.z < minz)
			minz = tempPoint.z;
		if (tempPoint.x > maxx)
			maxx = tempPoint.x;
		if (tempPoint.y > maxy)
			maxy = tempPoint.y;
		if (tempPoint.z > maxz)
			maxz = tempPoint.z;
	}

	DBSTREAM3( cdebug << "QColBox::Bound minx <" << minx << ">, miny <" << miny << ">, minz <" << minz << "> maxx <" << )
   	DBSTREAM3( maxx << ">, maxy <" << maxy << ">, maxz <" << maxz << ">" << endl; )

//	float masterScale = float(GetMasterScale(UNITS_METERS));
	float masterScale = 1.0;

	_min.x = minx / masterScale;
	_min.y = miny / masterScale;
	_min.z = minz / masterScale;
	_max.x = maxx / masterScale;
 	_max.y = maxy / masterScale;
	_max.z = maxz / masterScale;

	DBSTREAM3( cdebug << *this << endl; )

	AssertMessageBox(_min.x <= _max.x, "minX = " << _min.x << ", maxX = " << _max.x);
    AssertMessageBox(_min.y <= _max.y, "minY = " << _min.y << ", maxY = " << _max.y);
    AssertMessageBox(_min.z <= _max.z, "minZ = " << _min.z << ", maxZ = " << _max.z);
#else
	assert(0);
#endif
}

//============================================================================
void QColBox::Rotate(float angle, Point3& axis)
{
	Point3 tempPoints[8], new_min, new_max;

	for (int index=0; index<8; index++)
	{
		((index & 0x01)	? tempPoints[index].x = _min.x : tempPoints[index].x = _max.x);
		((index & 0x02)	? tempPoints[index].y = _min.y : tempPoints[index].y = _max.y);
		((index & 0x04)	? tempPoints[index].z = _min.z : tempPoints[index].z = _max.z);

//		tempPoints[index].Rotate(angle, axis);
#pragma message ("KTS " __FILE__ ": finish this")
		//tempPoints[index] = tempPoints[index] * RotAngleAxisMatrix(axis, angle);
		assert(0);


		if (tempPoints[index].x < new_min.x)
			new_min.x = tempPoints[index].x;
		if (tempPoints[index].y < new_min.y)
			new_min.y = tempPoints[index].y;
		if (tempPoints[index].z < new_min.z)
			new_min.z = tempPoints[index].z;
		if (tempPoints[index].x > new_max.x)
			new_max.x = tempPoints[index].x;
		if (tempPoints[index].y > new_max.y)
			new_max.y = tempPoints[index].y;
		if (tempPoints[index].z > new_max.z)
			new_max.z = tempPoints[index].z;
	}

	_max = new_max;
	_min = new_min;
}

//============================================================================

bool
QColBox::operator==(const QColBox& left) const
{
	return
	 (
		(_min == left._min) &&
		(_max == left._max)
	 );
}

//============================================================================
// Printing

std::ostream& operator<<(std::ostream& s, const QColBox &ColBox)
{
	s << "QColBox Dump" << endl;
	s << ColBox._min << ColBox._max;
	return s;
}

//============================================================================

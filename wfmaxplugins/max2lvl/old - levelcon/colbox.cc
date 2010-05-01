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

#include <stdio.h>
#include "qstring.hp"

#include <pclib/stdstrm.hp>
#include <pclib/hdump.hp>

#include "pigtool.h"
#include "level.hp"
#include "ColBox.hp"
#include <source\levelcon.h>		// included from velocity\source

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================

QColBox::QColBox()
{
}

//============================================================================

QColBox::QColBox(QPoint min, QPoint max)
{
	_min = min;
	_max = max;
    assert(_min.x() <= _max.x() && _min.y() <= _max.y() && _min.z() <= _max.x());
}

//============================================================================

QColBox::~QColBox()
{
}

//============================================================================

QColBox&
QColBox::operator+=(const QPoint& offset)							// add offset to all points in colbox
{
	_min += offset;
	_max += offset;
	return(*this);
}

//============================================================================

boolean
QColBox::InsideCheck(const QPoint point) const
{
	DBSTREAM3( cdebug << "QColBox::InsideCheck: checking if "; )
	DBSTREAM3( cdebug << point << "is inside of " << *this; )

	// kts changed to not include edges (removed =)
	// kts changed to include edges (added =)
	if(
		point.x() >= _min.x() && point.x() <= _max.x() &&
		point.y() >= _min.y() && point.y() <= _max.y() &&
		point.z() >= _min.z() && point.z() <= _max.z()
	  )
	 {
		DBSTREAM3( cdebug << "   True" << endl; )
		return(boolean::BOOLTRUE);
	 }
	DBSTREAM3( cdebug << "   False" << endl; )
	return(boolean::BOOLFALSE);
}

//============================================================================

inline int
colboxabs(int a)
{
	return(a<0?-a:a);
}

double
QColBox::GetVolume(void) const
{
	double xSize,ySize,zSize;

	xSize = BrScalarToFloat(colboxabs(_min.x() - _max.x()));
	ySize = BrScalarToFloat(colboxabs(_min.y() - _max.y()));
	zSize = BrScalarToFloat(colboxabs(_min.z() - _max.z()));
	assert(xSize >= 0);
	assert(ySize >= 0);
	assert(zSize >= 0);
	double result = xSize * ySize * zSize;
//	cdebug << "QColBox::GetVolume: xSize = <" << xSize << ">, ySize = <" << ySize << ">, zSize = <" << zSize << ">, volume = <" << result << ">" << endl;
	return(result);
}

//============================================================================

size_t
QColBox::SizeOfOnDisk(void) const
{
	return(sizeof(_CollisionRectOnDisk));
}

//============================================================================
// Write out to the on disk structure

void
QColBox::Write(_CollisionRectOnDisk* destColBox) const
{
	destColBox->minX = _min.x();
	destColBox->minY = _min.y();
	destColBox->minZ = _min.z();
	destColBox->maxX = _max.x();
	destColBox->maxY = _max.y();
	destColBox->maxZ = _max.z();

	DBSTREAM3( cdebug << "QColBox::Write: writting colbox " << endl; )
	DBSTREAM3( cdebug << "  MinX: <" << BrScalarToFloat(destColBox->minX) << ">, MinY: <" << BrScalarToFloat(destColBox->minY) << "> MinZ: <" << BrScalarToFloat(destColBox->minZ) << ">" << endl; )
 	DBSTREAM3( cdebug << "  MaxX: <" << BrScalarToFloat(destColBox->maxX) << ">, MaxY: <" << BrScalarToFloat(destColBox->maxY) << "> MaxZ: <" << BrScalarToFloat(destColBox->maxZ) << ">" << endl; )
}

//============================================================================

void
QColBox::Bound(const mesh3ds& mesh, const kfmesh3ds& kfmesh)
{
	float minx = mesh.vertexarray[0].x;
	float miny = mesh.vertexarray[0].y;
	float minz = mesh.vertexarray[0].z;
	float maxx = mesh.vertexarray[0].x;
	float maxy = mesh.vertexarray[0].y;
	float maxz = mesh.vertexarray[0].z;

	for(int index=0; index < mesh.nvertices; index++)
	 {
		if(mesh.vertexarray[index].x < minx)
			minx = mesh.vertexarray[index].x;
		if(mesh.vertexarray[index].y < miny)
			miny = mesh.vertexarray[index].y;
		if(mesh.vertexarray[index].z < minz)
			minz = mesh.vertexarray[index].z;
		if(mesh.vertexarray[index].x > maxx)
			maxx = mesh.vertexarray[index].x;
		if(mesh.vertexarray[index].y > maxy)
			maxy = mesh.vertexarray[index].y;
		if(mesh.vertexarray[index].z > maxz)
			maxz = mesh.vertexarray[index].z;

//		cdebug << "QColBox::Bound checking minx <" << mesh.vertexarray[index].x << ">, miny <" << mesh.vertexarray[index].y << ">, minz <" << mesh.vertexarray[index].z << ">" << endl;
	 }

	DBSTREAM3( cdebug << "QColBox::Bound minx <" << minx << ">, miny <" << miny << ">, minz <" << minz << "> maxx <" << )
   	DBSTREAM3( maxx << ">, maxy <" << maxy << ">, maxz <" << maxz << ">" << endl; )

	_min.x(BrFloatToScalar(FLOAT2VELOCITY(minx-kfmesh.pos[0].x)));
	_min.y(BrFloatToScalar(FLOAT2VELOCITY(miny-kfmesh.pos[0].y)));
	_min.z(BrFloatToScalar(FLOAT2VELOCITY(minz-kfmesh.pos[0].z)));
	_max.x(BrFloatToScalar(FLOAT2VELOCITY(maxx-kfmesh.pos[0].x)));
 	_max.y(BrFloatToScalar(FLOAT2VELOCITY(maxy-kfmesh.pos[0].y)));
	_max.z(BrFloatToScalar(FLOAT2VELOCITY(maxz-kfmesh.pos[0].z)));

	DBSTREAM3( cdebug << *this << endl; )

    assert(_min.x() <= _max.x() && _min.y() <= _max.y() && _min.z() <= _max.z());
}

//============================================================================
void QColBox::Rotate(float angle, QPoint& axis)
{
	QPoint tempPoints[8], new_min, new_max;

	for (int index=0; index<8; index++)
	{
		((index & 0x01)	? tempPoints[index].x(_min.x()) : tempPoints[index].x(_max.x()));
		((index & 0x02)	? tempPoints[index].y(_min.y()) : tempPoints[index].y(_max.y()));
		((index & 0x04)	? tempPoints[index].z(_min.z()) : tempPoints[index].z(_max.z()));

		tempPoints[index].Rotate(angle, axis);

		if (tempPoints[index].x() < new_min.x())
			new_min.x(tempPoints[index].x());
		if (tempPoints[index].y() < new_min.y())
			new_min.y(tempPoints[index].y());
		if (tempPoints[index].z() < new_min.z())
			new_min.z(tempPoints[index].z());
		if (tempPoints[index].x() > new_max.x())
			new_max.x(tempPoints[index].x());
		if (tempPoints[index].y() > new_max.y())
			new_max.y(tempPoints[index].y());
		if (tempPoints[index].z() > new_max.z())
			new_max.z(tempPoints[index].z());
	}

	_max = new_max;
	_min = new_min;
}

//============================================================================
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

ostream& operator<<(ostream& s, const QColBox &ColBox)
{
	s << "QColBox Dump" << endl;
	s << ColBox._min << ColBox._max;
	return s;
}

//============================================================================


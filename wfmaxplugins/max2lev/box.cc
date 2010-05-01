// Box.cpp: implementation of the Box class.
//
//////////////////////////////////////////////////////////////////////

#include "box.h"
//#include <pigsys/assert.h>

//============================================================================

Box::Box()
{
	_min = _max = Point3( 0.0, 0.0, 0.0 );
}

//============================================================================

Box::Box( Point3 min, Point3 max )
{
	_min = min;
	_max = max;
    assert( _min.x <= _max.x && _min.y <= _max.y && _min.z <= _max.x );
}

//============================================================================

Box::~Box()
{
}

//============================================================================

Box&
Box::operator+=( const Point3& offset )							// add offset to all points in colbox
{
	_min += offset;
	_max += offset;
	return *this;
}

//============================================================================

Box&
Box::operator-=( const Point3& offset )							// add offset to all points in colbox
{
	_min -= offset;
	_max -= offset;
	return *this;
}

//============================================================================

bool
Box::InsideCheck( const Point3& point ) const
{
	return
		( point.x >= _min.x && point.x <= _max.x ) &&
		( point.y >= _min.y && point.y <= _max.y ) &&
		( point.z >= _min.z && point.z <= _max.z );
}

//============================================================================

static inline double
abs( double a )
{
	return a < 0 ? -a : a;
}

double
Box::GetVolume() const
{
	double xSize = abs( _min.x - _max.x );
	double ySize = abs( _min.y - _max.y );
	double zSize = abs( _min.z - _max.z );

	assert( xSize >= 0.0 );
	assert( ySize >= 0.0 );
	assert( zSize >= 0.0 );

	return xSize * ySize * zSize;
}

//============================================================================

void
Box::Bound( const Mesh& mesh, Matrix3& rotMatrix )
{
	//AssertMessageBox(mesh.numVerts > 0, "mesh doesn't have any vertices" );
	assert( mesh.numVerts > 0 );

	Point3 tempPoint = mesh.verts[0] * rotMatrix;
	double minx = tempPoint.x;
	double miny = tempPoint.y;
	double minz = tempPoint.z;
	double maxx = tempPoint.x;
	double maxy = tempPoint.y;
	double maxz = tempPoint.z;

	for ( int index=0; index < mesh.numVerts; ++index )
	{
		tempPoint = mesh.verts[index] * rotMatrix;
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

	//double masterScale = GetMasterScale( UNITS_METERS );
	double masterScale = 1.0;
	_min.x = minx / masterScale;
	_min.y = miny / masterScale;
	_min.z = minz / masterScale;
	_max.x = maxx / masterScale;
 	_max.y = maxy / masterScale;
	_max.z = maxz / masterScale;
}

//============================================================================
void
Box::Rotate( double angle, Point3& axis )
{
	Point3 tempPoints[8], new_min, new_max;

	for ( int index=0; index<8; ++index )
	{
		((index & 0x01)	? tempPoints[index].x = _min.x : tempPoints[index].x = _max.x);
		((index & 0x02)	? tempPoints[index].y = _min.y : tempPoints[index].y = _max.y);
		((index & 0x04)	? tempPoints[index].z = _min.z : tempPoints[index].z = _max.z);

//		tempPoints[index].Rotate(angle, axis);
		tempPoints[index] = tempPoints[index] * RotAngleAxisMatrix(axis, angle);

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
Box::operator==( const Box& left ) const
{
	return ( _min == left._min ) && ( _max == left._max );
}

//============================================================================
// Printing

ostream& operator<<(ostream& s, const Box& box )
{
	s << "Box ";	// << box._min << '-' << box._max;
	return s;
}

//============================================================================

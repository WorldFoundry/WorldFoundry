//============================================================================
// Point.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
/* Documentation:

	Abstract:
			3D Points and vectors
	History:
			Created	07-31-95 02:11pm Kevin T. Seghetti

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

#include "pigtool.h"
#include "Point.hp"
#include <source\levelcon.h>		// included from velocity\source

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//============================================================================

QPoint::QPoint()
{
	_x = _y = _z = 0;
}

//============================================================================

QPoint::QPoint( br_scalar newX, br_scalar newY, br_scalar newZ)
{
	_x = newX;
	_y = newY;
	_z = newZ;
}

//============================================================================

QPoint::~QPoint()
{
}

//============================================================================

QPoint&
QPoint::operator-=(const QPoint& left)
{
	_x -= left._x;
	_y -= left._y;
	_z -= left._z;
	return(*this);
}

//============================================================================

QPoint&
QPoint::operator+=(const QPoint& left)
{
	_x += left._x;
	_y += left._y;
	_z += left._z;
	return(*this);
}

//============================================================================

QPoint::operator==(const QPoint& left) const
{
	return
	 (
		(_x == left._x) &&
		(_y == left._y) &&
		(_z == left._z)
	 );
}

//============================================================================
// QPoint::Rotate()
// Causes this Point to rotate itself 'angle' radians about the indicated axis.
void QPoint::Rotate(float rotation, QPoint& axis)
{
	br_angle	rot_angle;
	br_vector3	new_point, rot_axis;
	br_vector4	old_point;
	br_matrix34	rot_matrix;

	rot_angle = BrRadianToAngle(BrFloatToScalar(rotation));

	rot_axis.v[0] = axis._x;
	rot_axis.v[1] = axis._y;
	rot_axis.v[2] = axis._z;

	old_point.v[0] = _x;
	old_point.v[1] = _y;
	old_point.v[2] = _z;
	old_point.v[3] = BrFloatToScalar(1.0);

	BrMatrix34Rotate(&rot_matrix, rot_angle, &rot_axis);
	BrMatrix34Apply(&new_point, &old_point, &rot_matrix);

	_x = new_point.v[0];
	_y = new_point.v[1];
	_z = new_point.v[2];
}


//============================================================================
// Printing

ostream& operator<<(ostream& s, const QPoint &point)
{
	s << "x <" << BrScalarToFloat(point._x) << "> y <" << BrScalarToFloat(point._y) << "> z <" << BrScalarToFloat(point._z) << "> " << endl;
	return s;
}

//============================================================================

//============================================================================
// Point6.cpp:
// Copyright(c) 1997 Recombinant, Ltd.
//============================================================================

#include "global.hp"
#include "point6.hp"


//============================================================================

Point6::Point6()
{
	_pos = Point3(0,0,0);
	_rot = Quat(0.0,0.0,0.0,0.0);
}

//============================================================================

Point6::Point6( const Point3& newPos, const Quat& newRot )
{
	_pos = newPos;
	_rot = newRot;
}

//============================================================================

Point6::~Point6()
{
}

//============================================================================

Point6&
Point6::operator-=(const Point6& left)
{
	_pos -= left._pos;
#pragma message ("KTS " __FILE__ ": finish this")
	//_rot -= left._rot;
	assert(0);
	return(*this);
}

//============================================================================

Point6&
Point6::operator+=(const Point6& left)
{
	_pos += left._pos;
#pragma message ("KTS " __FILE__ ": finish this")
	//_rot *= left._rot;
	assert(0);
	return(*this);
}

//============================================================================

Point6&
Point6::operator=(const Point6& left)
{
	_pos = left._pos;
	_rot = left._rot;
	return(*this);
}

//============================================================================

bool
Point6::operator==(const Point6& left) const
{
	assert(0);	
	return
	 (
		(_pos == left._pos) 
#pragma message ("KTS " __FILE__ ": finish this")
//		&& (_rot == left._rot)
	 );
}

//============================================================================
// QPoint::Rotate()
// Causes this Point to rotate itself 'angle' radians about the indicated axis.
//void QPoint::Rotate(float rotation, QPoint& axis)
//{
//	br_angle	rot_angle;
//	br_vector3	new_point, rot_axis;
//	br_vector4	old_point;
//	br_matrix34	rot_matrix;

//	rot_angle = BrRadianToAngle(WF_FLOAT_TO_SCALAR(rotation));

//	rot_axis.v[0] = axis._x;
//	rot_axis.v[1] = axis._y;
//	rot_axis.v[2] = axis._z;

//	old_point.v[0] = _x;
//	old_point.v[1] = _y;
//	old_point.v[2] = _z;
//	old_point.v[3] = WF_FLOAT_TO_SCALAR(1.0);

//	BrMatrix34Rotate(&rot_matrix, rot_angle, &rot_axis);
//	BrMatrix34Apply(&new_point, &old_point, &rot_matrix);

//	_x = new_point.v[0];
//	_y = new_point.v[1];
//	_z = new_point.v[2];
//}


//============================================================================
// Printing

ostream& operator<<(ostream& s, const Point6 &thePoint)
{
	float angle[3];
#pragma message ("KTS " __FILE__ ": finish this")
	//QuatToEuler((Quat&)thePoint._rot, angle);
	assert(0);
	s << "Translation: " << thePoint._pos << "; Rotation: x = " << angle[0] << ", y = " << angle[1] << ", z = " << angle[2] << endl;
	return s;
}

//============================================================================

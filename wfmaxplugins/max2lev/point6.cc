//============================================================================
// Point6.cpp:
// Copyright(c) 1997 Recombinant, Ltd.
//============================================================================

#include "Point6.hpp"

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
	_rot -= left._rot;
	return(*this);
}

//============================================================================

Point6&
Point6::operator+=(const Point6& left)
{
	_pos += left._pos;
	_rot *= left._rot;
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

Point6::operator==(const Point6& left) const
{
	return
	 (
		(_pos == left._pos) &&
		(_rot == left._rot)
	 );
}

//============================================================================
// Printing

ostream& operator<<(ostream& s, const Point6 &thePoint)
{
	float angle[3];
	QuatToEuler((Quat&)thePoint._rot, angle);
#pragma message( "FIX" )
//?	s << "Translation: " << thePoint._pos << "; Rotation: x = " << angle[0] << ", y = " << angle[1] << ", z = " << angle[2] << endl;
	return s;
}

//============================================================================

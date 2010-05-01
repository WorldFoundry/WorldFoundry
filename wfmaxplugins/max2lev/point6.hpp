//============================================================================
// Point6.hp:
// Copyright(c) 1997 Recombinant Ltd.
//============================================================================
// use only once insurance

#ifndef _Point6_HP
#define _Point6_HP

//============================================================================

#include <max.h>
#include <iostream.h>
//#include "pigtool.h"

//============================================================================

class Point6
{
public:
	Point6();
	~Point6();
	Point6( const Point3& newPos, const Quat& newRot=Quat(0.0, 0.0, 0.0, 0.0) );

	Point3 pos(void) const { return(_pos); }
	Point3 pos(Point3& newPoint)  { _pos = newPoint; return(_pos); }
	Quat   rot(void) const { return(_rot); }
	Quat   rot(Quat& newRot)	  { _rot = newRot; return(_rot); }

	void clearPos(void) { _pos = Point3(0.0, 0.0, 0.0); }
	void clearRot(void) { _rot = Quat(0.0, 0.0, 0.0, 0.0); }
	void clear(void)    { _pos = Point3(0.0, 0.0, 0.0); _rot = Quat(0.0, 0.0, 0.0, 0.0); }

	// Printing
	friend ostream& operator<<(ostream& s, const Point6 &o);

	Point6& operator+=(const Point6& left);
	Point6& operator-=(const Point6& left);
	Point6& operator=(const Point6& left);
	operator==(const Point6& left) const;
private:
	Point3	_pos;
	Quat	_rot;
};

//============================================================================
#endif
//============================================================================

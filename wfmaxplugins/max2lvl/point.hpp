//============================================================================
// Point.hp:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
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
// use only once insurance

#ifndef _Point_HP
#define _Point_HP

//============================================================================

#include "global.hpp"

#include <iostream.h>
//#include "pigtool.h"

//============================================================================

class QPoint
{
public:
	QPoint();
	~QPoint();
	QPoint( fixed32 newX, fixed32 newY, fixed32 newZ);

	fixed32 x(void) const { return(_x); }
	fixed32 x(fixed32 x)  { _x = x; return(_x); }
	fixed32 y(void) const { return(_y); }
	fixed32 y(fixed32 y) { _y = y; return(_y); }
	fixed32 z(void) const { return(_z); }
	fixed32 z(fixed32 z) { _z = z; return(_z); }

	// Added for colbox rotation
	void Rotate(float rotation, QPoint& axis);

	// Printing
	friend ostream& operator<<(ostream& s, const QPoint &o);

	QPoint  operator+(const QPoint& a) const { QPoint res = a; res += *this; return res; }
	QPoint  operator-(const QPoint& a) const { QPoint res = a; res -= *this; return res; }

	QPoint& operator+=(const QPoint& left);
	QPoint& operator-=(const QPoint& left);
	operator==(const QPoint& left) const;
private:
	fixed32 _x,_y,_z;
};

//============================================================================
#endif
//============================================================================

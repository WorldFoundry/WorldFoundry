// Box.h: interface for the Box class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOX_H__935F8800_A1A8_11D1_93D2_00C0F0169F1D__INCLUDED_)
#define AFX_BOX_H__935F8800_A1A8_11D1_93D2_00C0F0169F1D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <max.h>
//#include "global.hpp"
//#include "point.hpp"
//#include <source/levelcon.h>		// included from velocity\source
//#include <stdio.h>

class Box
{
public:
	Box();
	Box( Point3 min, Point3 max );
	~Box();

	void Bound( const Mesh& mesh, Matrix3& rotMatrix );	// read from array of 3d points, and create a bounding box
	bool InsideCheck( const Point3& point ) const;
//	void Write(_CollisionRectOnDisk* destBuffer) const;
//	size_t SizeOfOnDisk() const;			// returns size in bytes of on-disk representation

	Point3 GetMin() const { return _min; }
	Point3 GetMax()	const { return _max; }
	Point3 SetMin(Point3 newMin)	{ _min = newMin; return _min; }
	Point3 SetMax(Point3 newMax)	{ _max = newMax; return _max; }

	double GetVolume() const;					// approximatly
	void Rotate( double angle, Point3& axis );

	// Printing
	friend ostream& operator<<( ostream& s, const Box& o );
	operator==(const Box& left) const;
	Box& operator+=( const Point3& offset );							// add offset to all points in colbox
	Box& operator-=( const Point3& offset );

protected:
	Point3 _min;
	Point3 _max;
};

#endif // !defined(AFX_BOX_H__935F8800_A1A8_11D1_93D2_00C0F0169F1D__INCLUDED_)

//============================================================================
// path.hp:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
//============================================================================
/* Documentation:

	Abstract:
			in memory representation of an object path
	History:
			Created	06-14-95 05:05pm Kevin T. Seghetti

	Class Hierarchy:
			none

	Dependancies:
		PIGS, BRender, STDIO
	Restrictions:

	Example:
*/
//============================================================================
// use only once insurance

#ifndef _path_HP
#define _path_HP

//============================================================================

//#include <stl/vector.h>
#include <max.h>
//#include <pigsys/pigsys.h>
#include "point6.hpp"
#include "channel.hpp"

//============================================================================
// in memory representation of a 3D path

class Path
{
public:
	Path();
	Path(const Point3& basePos, const Quat& baseRot);				// relative path
	~Path();

	void AddPositionKey(TimeValue time, Point3& position);
	void AddRotationKey(TimeValue time, Quat& rotation);
	Point3 GetPosition(TimeValue time) const;
	Quat GetRotation(TimeValue time) const;
	void AddPositionOffset(const Point3& offset);

	// Printing
//	friend ostream& operator<<(ostream& s, const Path &o);
//	operator==(const Path& left) const { return(0); }
	_IffWriter& _print( _IffWriter& _iff );

protected:
	Point3 basePosition;				// base XYZ for (relative) path
	Quat baseRotation;
	Channel positionXChannel;				
	Channel positionYChannel;
	Channel positionZChannel;
	Channel rotationAChannel;
	Channel rotationBChannel;
	Channel rotationCChannel;
};


inline _IffWriter&
operator << ( _IffWriter& iff, Path& path )
{
	return path._print( iff );
}

//============================================================================
#endif
//============================================================================

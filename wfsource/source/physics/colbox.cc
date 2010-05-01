//==============================================================================
// colbox.cc:
// Copyright ( c ) 1994,95,96,97,98,99 World Foundry Group  
// Part of the World Foundry 3D video game engine/production environment
// for more information about World Foundry, see www.worldfoundry.org
//==============================================================================
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// Version 2 as published by the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// or see www.fsf.org

//=============================================================================
// Description: The ColBox class and subclasses describe different collision
//              spaces and calculate whether a particular colbox collides with
//              another.
// Original Author: Ann Mei Chang
// rewrite: Kevin T. Seghetti, 2/25/96 7:45AM
//==============================================================================

#define _COLBOX_CC
#include "colbox.hp"
//#include "gamestrm.hp"

//#if DEBUG > 0
//#	include "actor.hp"	// for validation ONLY
//#endif

//==============================================================================

ColBox::ColBox( const Vector3& min, const Vector3& max)
	: _minPoint(min), _maxPoint(max)
{
}

//==============================================================================
// Expand colbox by passed in vector (used to generate colboxes
// which contain starting and ending position of a moving object)

void
ColBox::Expand(const Vector3& delta)
{
#if DO_ASSERTIONS
	// Make sure no velocity component will overflow Scalar
	const Scalar MAXVEL = SCALAR_CONSTANT(32767);
	AssertMsg(((delta.X() < MAXVEL) && (delta.X() > -MAXVEL) &&
			   (delta.Y() < MAXVEL) && (delta.Y() > -MAXVEL) &&
			   (delta.Z() < MAXVEL) && (delta.Z() > -MAXVEL) ), "An actor has acheived LUDICROUS SPEED.");
#endif

	( delta.X() > Scalar::zero )?_maxPoint.SetX(Max().X() + delta.X()):_minPoint.SetX(Min().X() + delta.X());
	( delta.Y() > Scalar::zero )?_maxPoint.SetY(Max().Y() + delta.Y()):_minPoint.SetY(Min().Y() + delta.Y());
	( delta.Z() > Scalar::zero )?_maxPoint.SetZ(Max().Z() + delta.Z()):_minPoint.SetZ(Min().Z() + delta.Z());
}

//==============================================================================
// this = object 1 colbox, mypos = position of object 1 in world
// object = object2, curpos = position of object2 in the world
// kts 03-29-96 11:50am, if one edge is coincident, no collision

#pragma message("ColBox::CheckCollision() should be changed to not allow coincident edges!")

bool
ColBox::CheckCollision(const Vector3& mypos, const ColBox& object, const Vector3& objpos) const
{
   Vector3 min = Min(mypos);
   Vector3 max = Max(mypos);
   Vector3 omin = object.Min(objpos);
   Vector3 omax = object.Max(objpos);
	if ((max.Z() <= omin.Z()) ||
		(min.Z() >= omax.Z()))
		return false; // one object past the other
	if ((max.X() <= omin.X()) ||
		(min.X() >= omax.X()))
		return false; // one object to side of other
	if ((max.Y() <= omin.Y()) ||
		(min.Y() >= omax.Y()))
		return false; // one object above the other
	return true;
}

//==============================================================================
// this = object 1 colbox, mypos = position of object 1 in world
// endpoint1,endpoint2 = the vector to collide against (in global space)

bool
ColBox::CheckCollision(const Vector3& mypos, const Vector3& endpoint1, const Vector3& endpoint2) const
{
   Vector3 min = Min(mypos);
   Vector3 max = Max(mypos);

	if ( (endpoint1.X() < min.X()) && (endpoint2.X() < min.X()) )
		return false;          
	if ( (endpoint1.X() > max.X()) && (endpoint2.X() > max.X()) )
		return false;          
	if ( (endpoint1.Y() < min.Y()) && (endpoint2.Y() < min.Y()) )
		return false;          
	if ( (endpoint1.Y() > max.Y()) && (endpoint2.Y() > max.Y()) )
		return false;          
	if ( (endpoint1.Z() < min.Z()) && (endpoint2.Z() < min.Z()) )
		return false;          
	if ( (endpoint1.Z() > max.Z()) && (endpoint2.Z() > max.Z()) )
		return false;

	return true;
}

//==============================================================================

bool
ColBox::CheckCollision(const Vector3& mypos, const Vector3& endpoint) const
{
   Vector3 min = Min(mypos);
   Vector3 max = Max(mypos);
	if ( endpoint.X() < min.X() )
		return false;
	if ( endpoint.X() > max.X() )
		return false;
	if ( endpoint.Y() < min.Y() )
		return false;
	if ( endpoint.Y() > max.Y() )
		return false;
	if ( endpoint.Z() < min.Z() )
		return false;
	if ( endpoint.Z() > max.Z() )
		return false;

	return true;
}

//==============================================================================
// this = object 1 colbox, mypos = position of object 1 in world
// object = object2, curpos = position of object2 in the world
// returns true if object2 is entirely contained in object1

bool
ColBox::CheckSurrounds(const Vector3& mypos, const ColBox& object, const Vector3& objpos) const
{
   Vector3 min = Min(mypos);
   Vector3 max = Max(mypos);
   Vector3 omin = object.Min(objpos);
   Vector3 omax = object.Max(objpos);

	if( (max.X() < omax.X()) ||
		(min.X() > omin.X()) )
		 return false;			// not inside
	if( (max.Z() < omax.Z()) ||
		(min.Z() > omin.Z()) )
		 return false;			// not inside
	if( (max.Y() < omax.Y()) ||
		(min.Y() > omin.Y()) )
		 return false;			// not inside
	return true;
}

//==============================================================================
// this = object 1 qspace, mypos = position of object 1 in world
// object = object2, curpos = position of object2 in the world

bool
ColBox::CheckBelow(const Vector3& mypos, const ColBox& object, const Vector3& objpos) const
{
   Vector3 min = Min(mypos);
   Vector3 max = Max(mypos);
   Vector3 omin = object.Min(objpos);
   Vector3 omax = object.Max(objpos);

	if ((max.Y() < omin.Y()) ||
		(min.Y() > omax.Y()))
		return false; // one object past the other
	if ((max.X() < omin.X()) ||
		(min.X() > omax.X()))
		return false; // one object to side of other
	if (min.Z() < omax.Z())
		return false; // object is above me
	return true;
}

//==============================================================================

Vector3
ColBox::GetCenter( const Vector3& posCurrent ) const
{
   return posCurrent + (Min() + Max()) / Scalar::two;
}

//==============================================================================

#if SW_DBSTREAM >= 1

std::ostream&
Print(std::ostream& s, const ColBox& box, const Vector3& position)
{
	Vector3 min = box._minPoint + position;
	Vector3 max = box._maxPoint + position;
	s << "Min: " << min << std::endl;
	s << "                              Max: " << max;
	return(s);
}


std::ostream&
operator << ( std::ostream& s, const ColBox& box )
{
	s << "Min: " << box._minPoint << std::endl;
	s << "                              Max: " << box._maxPoint;
	return s;
}

#endif // SW_DBSTREAM >= 1
//==============================================================================

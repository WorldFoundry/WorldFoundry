//==============================================================================
// colspace.cc:
//==============================================================================
// Copyright (c) 1997,99 World Foundry Group, 
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
//
//==============================================================================
//
// Description: The ColSpace class encapsulates an Actor's collision space, and
//              offers methods to check collision at both fine (axis aligned box)
//				and coarse levels of detail.  The only fine-grained collision
//				space representation currently supported is a planar sloped surface.
//
// Original Author: Phil Torre, 07/03/97
//
//==============================================================================

#define _COLSPACE_CC
#include "colspace.hp"
//#include <cpplib/stdstrm.hp>
DBSTREAM1( extern ostream_withassign( ccollision ); )
//#include "level.hp"

//==============================================================================

ColSpace::ColSpace()
	: _coarseColBox(Vector3::zero, Vector3::one),
	  _coarseExpandedColBox(Vector3::zero, Vector3::one)
{

}

//==============================================================================

ColSpace::ColSpace( const Vector3& min, const Vector3& max)
	: _coarseColBox(min, max), _coarseExpandedColBox(min, max)
{

}

//==============================================================================

ColSpace::ColSpace(const ColBox& coarseSpace)
	: _coarseColBox(coarseSpace), _coarseExpandedColBox(coarseSpace)
{

}

//==============================================================================

bool
ColSpace::CheckCollisionWithSlope(const Vector3& mypos, const ColSpace& object, const Vector3& objpos) const
{
	DBSTREAM3( ccollision << "Checking colSpace vs. sloped colspace..." << std::endl; )
	// Check all eight points of the other colbox against our sloped plane.
	// If any point is closer than _slope.D(), collision occurred.

	assert(ContainsSlope());	// make sure we're doing this in the right order!
	Scalar distance, vertexX, vertexY, vertexZ;
	Vector3 xform = objpos - mypos;		// transform to "my" local space

	for (int index=0; index < 8; index++)	// iterate over all points of the colbox
	{
      Vector3 omin = object.Min(xform);
      Vector3 omax = object.Max(xform);

		(index & 0x01) ? (vertexX = omax.X()) : (vertexX = omin.X());
		(index & 0x02) ? (vertexY = omax.Y()) : (vertexY = omin.Y());
		(index & 0x04) ? (vertexZ = omax.Z()) : (vertexZ = omin.Z());
		distance = (_slope.A() * vertexX) + (_slope.B() * vertexY) + (_slope.C() * vertexZ) + _slope.D();

		if (distance < Scalar::zero)
		{
			DBSTREAM3( ccollision << "HIT! Distance = " << distance << std::endl; )
			return true;
		}
	}

	DBSTREAM3( ccollision << "Missed." << std::endl; )
	return false;
}

//==============================================================================

bool
ColSpace::PremoveCollisionCheckWithSlope(const Vector3& mypos, const ColSpace& object, const Vector3& objpos) const
{
	DBSTREAM3( ccollision << "Checking colSpace vs. sloped colspace..." << std::endl; )
	// Check all eight points of the other colbox against our sloped plane.
	// If any point is closer than _slope.D(), collision occurred.

	assert(ContainsSlope());	// make sure we're doing this in the right order!
	Scalar distance, vertexX, vertexY, vertexZ;
	Vector3 xform = objpos - mypos;		// transform to "my" local space

	for (int index=0; index < 8; index++)	// iterate over all points of the colbox
	{
      Vector3 omin = object.UnExpMin(xform);
      Vector3 omax = object.UnExpMax(xform);

		(index & 0x01) ? (vertexX = omax.X()) : (vertexX = omin.X());
		(index & 0x02) ? (vertexY = omax.Y()) : (vertexY = omin.Y());
		(index & 0x04) ? (vertexZ = omax.Z()) : (vertexZ = omin.Z());

		// If this vertex is outside of our colbox, don't do the slope math
		if (_coarseColBox.CheckCollision( Vector3::zero, Vector3(vertexX, vertexY, vertexZ)) )
		{
			distance = (_slope.A() * vertexX) + (_slope.B() * vertexY) + (_slope.C() * vertexZ) + _slope.D();

			if (distance < Scalar::zero)
			{
				DBSTREAM3( ccollision << "HIT! Distance = " << distance << ", plane offset = " << _slope.D() << std::endl; )
				return true;
			}
		}
	}

	DBSTREAM3( ccollision << "Missed." << std::endl; )
	return false;
}

//==============================================================================

bool
ColSpace::CheckCollisionWithSlope(const Vector3& /*mypos*/, const Vector3& /*endpoint1 */, const Vector3& /*endpoint2*/ ) const
{

	DBSTREAM1( cerror << "Line segment vs. sloped ColSpace is UNIMPLIMENTED!" << std::endl; )
	return true;
}

//==============================================================================

bool
ColSpace::CheckCollisionWithSlope(const Vector3& /* mypos */, const Vector3& /*endpoint */) const
{

	DBSTREAM1( cerror << "Point vs. sloped ColSpace is UNIMPLIMENTED!" << std::endl; )
	return true;
}

//==============================================================================

Scalar
ColSpace::TimeToHitSlope(const Vector3& mypos, const ColSpace& object, const Vector3& objpos, const Vector3& objVel, const Scalar deltaT) const
{
	// Compute the distance between the closest point of the other object and our slope

	assert(ContainsSlope());	// make sure we're doing this in the right order!
	Scalar distanceInitial, Xinitial, Yinitial, Zinitial, distanceFinal, Xfinal, Yfinal, Zfinal;
	Scalar distanceOffset;
	Scalar time(Scalar::max), tempTime(Scalar::zero);
	Vector3 xform = objpos - mypos;		// transform to "my" local space
//	Scalar deltaT = theLevel->getDeltaClock();

	for (int index=0; index < 8; index++)	// iterate over all points of the colbox
	{
		if (index & 0x01)
		{
			Xinitial = object.UnExpMax(xform).X();
			if (objVel.X() > Scalar::zero)
				Xfinal = Xinitial + (objVel.X() * deltaT);
			else
				Xfinal = object.Max(xform).X();
		}
		else
		{
			Xinitial = object.UnExpMin(xform).X();
			if (objVel.X() < Scalar::zero)
				Xfinal = Xinitial + (objVel.X() * deltaT);
			else
				Xfinal = object.Min(xform).X();
		}

		if (index & 0x02)
		{
			Yinitial = object.UnExpMax(xform).Y();
			if (objVel.Y() > Scalar::zero)
				Yfinal = Yinitial + (objVel.Y() * deltaT);
			else
				Yfinal = object.Max(xform).Y();
		}
		else
		{
			Yinitial = object.UnExpMin(xform).Y();
			if (objVel.Y() < Scalar::zero)
				Yfinal = Yinitial + (objVel.Y() * deltaT);
			else
				Yfinal = object.Min(xform).Y();
		}

		if (index & 0x04)
		{
			Zinitial = object.UnExpMax(xform).Z();
			if (objVel.Z() > Scalar::zero)
				Zfinal = Zinitial + (objVel.Z() * deltaT);
			else
				Zfinal = object.Max(xform).Z();
		}
		else
		{
			Zinitial = object.UnExpMin(xform).Z();
			if (objVel.Z() < Scalar::zero)
				Zfinal = Zinitial + (objVel.Z() * deltaT);
			else
				Zfinal = object.Min(xform).Z();
		}


		// Don't bother doing the rest of the math if the point doesn't end up in our colbox
		if ( (_coarseExpandedColBox.CheckCollision(Vector3::zero, Vector3(Xfinal, Yfinal, Zfinal))) ||
			 (_coarseExpandedColBox.CheckCollision(Vector3::zero, Vector3(Xinitial, Yinitial, Zinitial))) )
		{
			distanceInitial = (_slope.A() * Xinitial) + (_slope.B() * Yinitial) + (_slope.C() * Zinitial);
			distanceFinal = (_slope.A() * Xfinal) + (_slope.B() * Yfinal) + (_slope.C() * Zfinal);
			distanceOffset = -_slope.D();

			if ( (distanceInitial >= distanceOffset) && (distanceFinal <= distanceOffset) )
			{
				tempTime = ((distanceInitial - distanceOffset) * deltaT) / (distanceInitial - distanceFinal);
				tempTime *= SCALAR_CONSTANT(0.99);

				if (tempTime < Scalar::zero)
					tempTime = Scalar::zero;

				assert(tempTime >= Scalar::zero);
				assert(tempTime <= deltaT);
				if (tempTime < time)
					time = tempTime;
			}
		}
	}

//	if (time == SCALAR_CONSTANT(32767))
//		return Scalar::zero;
//	else
		return time;
}

//==============================================================================

#if SW_DBSTREAM >= 1

std::ostream&
Print(std::ostream& s, const ColSpace& space, const Vector3& position)
{
	s << "  _colSpace: Coarse ColBox:   ";
	Print( s, space._coarseColBox, position);
	s << std::endl << "           Expanded ColBox:   ";
	Print( s, space._coarseExpandedColBox, position);
	s << std::endl;
	if (space.ContainsSlope())
		s << "  Slope: " << space._slope;
	return(s);
}

std::ostream&
operator << ( std::ostream& s, const ColSpace& space )
{
	s << "Coarse ColBox:   " << space._coarseColBox << std::endl;
	s << "Expanded ColBox: " << space._coarseExpandedColBox;
	return s;
}

#endif // SW_DBSTREAM >= 1

//==============================================================================


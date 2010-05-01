//==============================================================================
// physical.cc:
// Copyright (c) 1996,1997,1999,2000,20012002,2003 World Foundry Group  
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

//==============================================================================
// Description: The PhysicalAttributes class encapsulates the objects position
//   and orientation, collision boxes, and velocity vectors
// Original Author: Kevin T. Seghetti / Phil Torre
//==============================================================================

#define _PHYSICAL_CC

#include <physics/physical.hp>
#include <physics/physicalobject.hp>
#include <physics/colbox.hp>
#include <math/vector3.hp>
#include <math/matrix34.hp>
#include <cpplib/libstrm.hp>

//==============================================================================

#if defined PHYSICS_ENGINE_ODE
#include "ode/physical.cc"
#elif defined PHYSICS_ENGINE_WF
#include "wf/physical.cc"
#else
#error physics engine not defined! 
#endif

#define DO_3RD_CHECK
                                  
//==============================================================================

void
PhysicalAttributes::RotateAboutAxis( const Vector3& axis, const Angle rotation )
{
	if (rotation != Angle::zero)
	{
		Angle rotZ(Angle::zero);
		if (axis.Y() != Scalar::zero )
			rotZ = axis.X().ATan2(axis.Y());			// Rotation required to put axis into XZ plane
		Angle rotY( Angle::zero - axis.Z().ASin() );	// Rotation then required to make axis colinear with X

		Matrix34 localMatrix(Rotation(), Vector3::zero);

		if (rotZ != Angle::zero)
			localMatrix = localMatrix * Matrix34( Euler(Angle::zero, Angle::zero, ( Angle::zero - rotZ )), Vector3::zero );	// null Z
		if (rotY != Angle::zero)
			localMatrix = localMatrix * Matrix34( Euler(Angle::zero, ( Angle::zero - rotY ), Angle::zero), Vector3::zero );		// null Y
		localMatrix = localMatrix * Matrix34( Euler(rotation, Angle::zero, Angle::zero), Vector3::zero );	// rotate along X
		if (rotY != Angle::zero)
			localMatrix = localMatrix * Matrix34( Euler(Angle::zero, rotY, Angle::zero), Vector3::zero );					// restore Y
		if (rotZ != Angle::zero)
			localMatrix = localMatrix * Matrix34( Euler(Angle::zero, Angle::zero, rotZ), Vector3::zero );					// restore Z

		// Decompose resultant matrix into an Euler triplet and store back into _orientation
		Vector3 lookAt( Vector3::unitX * localMatrix );
		Vector3 lookUp( Vector3::unitZ * localMatrix );
		rotZ = lookAt.X().ATan2( lookAt.Y() );
		lookAt.RotateZ( -rotZ );
		lookUp.RotateZ( -rotZ );
		rotY = lookAt.Z().ATan2( lookAt.X() ) - Angle::Revolution(SCALAR_CONSTANT(0.25));
		lookUp.RotateY( -rotY );
		Angle rotX( lookUp.Y().ATan2(lookUp.Z()) - Angle::Revolution(SCALAR_CONSTANT(0.25)));

      Euler euler(rotX,rotY,rotZ);
		SetRotation(euler);

//#if DO_ASSERTIONS
//		DBSTREAM3(
//		cscripting << "-------------------- PA::RotateAboutAxis() just ran -------------------------" << std::endl;
//		cscripting << "Axis of rotation: " << axis << "; Angle = " << rotation << std::endl;
//		cscripting << "New PA Orientation: " << _orientation.GetA() << ", " << _orientation.GetB() << ", " << _orientation.GetC() << std::endl;
//		Matrix34 dummyMatrix(_orientation, Vector3::zero);
//		cscripting << "New lookAt vector: " << Vector3( Vector3::unitX * dummyMatrix ) << std::endl << std::endl;
//		cscripting << "New lookUp vector: " << Vector3( Vector3::unitZ * dummyMatrix ) << std::endl << std::endl;
//		cscripting << "Transformed matrix:" << std::endl << localMatrix << std::endl;
//		cscripting << "Matrix generated from new Eulers:" << std::endl << dummyMatrix << std::endl;
//		)
//#endif
	}
}


//==============================================================================

bool
PhysicalAttributes::FullCollisionCheck(const PhysicalAttributes& other, const Clock& clock) const
{
      if (CheckCollision(other))
      {

          if (CheckCollision(other))
          {	// kts first tier (expanded collision boxes) passed
              DBSTREAM3( ccollision << "passed 1..."; )
              DBSTREAM3( ccollision << "other = " << other << std::endl 
                  << "\nobject Physical: " << *this
                  << std::endl);

#ifdef DO_3RD_CHECK
                  // Now do really slow, expensive check against the volumes of space actually
                  // occupied by the objects
                  return ThirdCollisionCheck(other, clock.Delta());
#else
                  return true;
#endif	// DO_3RD_CHECK
          }
      }
      return false;
}

//==============================================================================
// The third layer of collision checking deals with the case where diagonal movement
// (or a sloped surface) causes a false hit detection in the corner of the expanded
// colbox where the Actor never really was.
// The algorithm:  For the X axis alone, calculate what times the two objects enter
// and leave overlap.  Repeat for Y and Z.  Then, examine the overlap-start and
// overlap-end times; if there is ever a time for which all three axes are in overlap,
// then the objects really hit.
bool
PhysicalAttributes::ThirdCollisionCheck(const PhysicalAttributes& other, Scalar deltaT) const
{
	Scalar XOverlapStart, XOverlapEnd, YOverlapStart, YOverlapEnd, ZOverlapStart, ZOverlapEnd;

   DBSTREAM3(ccollision << "PA::ThirdCollisionCheck:" << std::endl; )
	if (ContainsSlope())
		return ThirdCollisionCheckWithSlope(other);

	if (other.ContainsSlope())
		return other.ThirdCollisionCheckWithSlope(*this);

	// Assume "we" are stationary and assign all velocity to other
	Vector3 relativeVelocity = other.MeanVelocity() - MeanVelocity();
	const ColSpace& colSpace1 = GetColSpace();
	const ColSpace& colSpace2 = other.GetColSpace();
	Vector3 pos2 = other.Position();

#if SW_DBSTREAM >= 3
   ccollision << " position = " << Position() << std::endl;
   ccollision << " colSpace1 = " << colSpace1 << std::endl;
   ccollision << " pos2 = " << pos2 << std::endl;
   ccollision << " colSpace2 = " << colSpace2 << std::endl;
   ccollision << " relativeVelocity = " << relativeVelocity << std::endl;
#endif

	// Calculate OverlapStart and OverlapEnd times
	// (Times returned are: 0 = start of frame, deltaT = end of frame)

   Vector3 min1 = colSpace1.UnExpMin(Position());
   Vector3 max1 = colSpace1.UnExpMax(Position());
   Vector3 min2 = colSpace2.UnExpMin(pos2);
   Vector3 max2 = colSpace2.UnExpMax(pos2);

	ComputeOverlapTimes( min1.X(), max1.X(),
						 min2.X(), max2.X() ,
						 relativeVelocity.X(), XOverlapStart, XOverlapEnd,deltaT);
	ComputeOverlapTimes( min1.Y(), max1.Y(),
						 min2.Y(), max2.Y(),
						 relativeVelocity.Y(), YOverlapStart, YOverlapEnd,deltaT);

	ComputeOverlapTimes( min1.Z(), max1.Z(),
						 min2.Z(), max2.Z(),
						 relativeVelocity.Z(), ZOverlapStart, ZOverlapEnd,deltaT);

	Scalar overlapStart = XOverlapStart.Max( YOverlapStart.Max( ZOverlapStart ) );
	Scalar overlapEnd   = XOverlapEnd.Min( YOverlapEnd.Min( ZOverlapEnd ) );

   DBSTREAM3( ccollision 
      << " X overlapStartTime = " << XOverlapStart << ", X overlapEndTime = " << XOverlapEnd
      << "\n Y overlapStartTime = " << YOverlapStart << ", Y overlapEndTime = " << YOverlapEnd 
      << "\n Z overlapStartTime = " << ZOverlapStart << ", Z overlapEndTime = " << ZOverlapEnd 
      << std::endl; )

   DBSTREAM3(ccollision << " overlapStart = " << overlapStart << std::endl; )
   DBSTREAM3(ccollision << " overlapEnd = " << overlapEnd << std::endl; )

	if ( (overlapStart >= deltaT) || (overlapEnd <= Scalar::zero) )
		return false;

	return true;
}

//==============================================================================
// Helper function for ThirdCollisionCheck()
void
PhysicalAttributes::ComputeOverlapTimes(Scalar actor1MinEdge, Scalar actor1MaxEdge,
							Scalar actor2MinEdge, Scalar actor2MaxEdge, Scalar relativeVelocity,
							Scalar& overlapStartTime, Scalar& overlapEndTime, Scalar deltaT) const
{

   DBSTREAM3(ccollision << "PA::ComputeOverlapTimes:" << std::endl; )
   DBSTREAM3(ccollision << " actor1MinEdge = " << actor1MinEdge << ",actor1MaxEdge = " << actor1MaxEdge
                        << ", actor2MinEdge = " << actor2MinEdge << ", actor2MaxEdge = " << actor2MaxEdge 
                        << ", relativeVelocity = " << relativeVelocity 
                        << ", deltaT = " << deltaT << std::endl;) 

	// Identify the 4 possible cases at frame start....
	// #1: actor2 completely to the "right" of actor1
	if (actor2MinEdge > actor1MaxEdge)
	{
		if (relativeVelocity < Scalar::zero)	// actor2 moving to the left
		{
			overlapStartTime = (actor1MaxEdge - actor2MinEdge) / relativeVelocity;
			overlapEndTime = (actor1MinEdge - actor2MaxEdge) / relativeVelocity;
         DBSTREAM3( ccollision << " PA::COT case 1:" << std::endl; )
			return;
		}
		else	// actor2 moving to the right, or stationary
		{
			overlapStartTime = deltaT + Scalar::one;	// this never happens
			overlapEndTime = deltaT + Scalar::one;
         DBSTREAM3( ccollision << " PA::COT case 2:" << std::endl; )
			return;
		}
	}
	// #2: actors partially overlapping
	else if ( ((actor2MinEdge < actor1MaxEdge) && (actor2MinEdge > actor1MinEdge) && (actor2MaxEdge > actor1MaxEdge)) ||
			  ((actor2MaxEdge < actor1MaxEdge) && (actor2MaxEdge > actor1MinEdge) && (actor2MinEdge < actor1MinEdge)) )
	{
		overlapStartTime = Scalar::zero;
		if (relativeVelocity < Scalar::zero)	// actor2 moving to the left
		{
			overlapEndTime = (actor1MinEdge - actor2MaxEdge) / relativeVelocity;
         DBSTREAM3( ccollision << " PA::COT case 3:" << std::endl; )
			return;
		}
		else if (relativeVelocity > Scalar::zero)	// actor2 moving to the right
		{
			overlapEndTime = (actor1MaxEdge - actor2MinEdge) / relativeVelocity;
         DBSTREAM3( ccollision << " PA::COT case 4:" << std::endl; )
			return;
		}
		else	// actor2 stationary
		{
			overlapEndTime = deltaT + Scalar::one;
         DBSTREAM3( ccollision << " PA::COT case 5:" << std::endl; )
			return;
		}
	}
	// #3: actor2 completely to the "left" of actor1
	else if (actor2MaxEdge < actor1MinEdge)
	{
		if (relativeVelocity > Scalar::zero)
		{
			overlapStartTime = (actor1MinEdge - actor2MaxEdge) / relativeVelocity;
			overlapEndTime = (actor1MaxEdge - actor2MinEdge) / relativeVelocity;
         DBSTREAM3( ccollision << " PA::COT case 6:" << std::endl; )
			return;
		}
		else
		{
			overlapStartTime = deltaT + Scalar::one;	// this never happens
			overlapEndTime = deltaT + Scalar::one;
         DBSTREAM3( ccollision << " PA::COT case 7:" << std::endl; )
			return;
		}
	}
	// #4: one actor totally overlapping the other
	else
	{
		overlapStartTime = Scalar::zero;
		if (relativeVelocity < Scalar::zero)
		{
			overlapEndTime = (actor1MinEdge - actor2MaxEdge) / relativeVelocity;
         DBSTREAM3( ccollision << " PA::COT case 8:" << std::endl; )
			return;
		}
		else if (relativeVelocity > Scalar::zero)
		{
			overlapEndTime = (actor1MaxEdge - actor2MinEdge) / relativeVelocity;
         DBSTREAM3( ccollision << " PA::COT case 9:" << std::endl; )
			return;
		}
		else
		{
			overlapEndTime = deltaT + Scalar::one;
         DBSTREAM3( ccollision << " PA::COT case 10:" << std::endl; )
			return;
		}
	}

	AssertMsg(0, "ComputeOverlapTimes() screwed up." << std::endl);
	return;		// make the compiler happy
}

//==============================================================================

bool
PhysicalAttributes::ThirdCollisionCheckWithSlope(const PhysicalAttributes& other) const
{
   DBSTREAM3(ccollision << "PA::ThirdCollisionCheckWithSlope:" << std::endl; )

	AssertMsg(!other.ContainsSlope(), "Slopes can't collide with other slopes yet!");

	// Assign all velocity to "other"
	Vector3 relativeVelocity = other.MeanVelocity() - MeanVelocity();
	const ColSpace& colSpace1 = GetColSpace();
	const ColSpace& colSpace2 = other.GetColSpace();

	// If other's UNexpanded box at its initial position is a hit, then we have a problem
//	assert(!colSpace1.PremoveCollisionCheckWithSlope(_position, colSpace2, other._position));

	// Now we know the other isn't hitting us at frame start.  Compute "tunnelling factor"
	// to see if they hit us after that
	Scalar tunnellingFactor = colSpace1.SlopeNormal().DotProduct(relativeVelocity);
	if (tunnellingFactor < SCALAR_CONSTANT(-0.01))
		return true;

	// Now check other's UNexpanded box at it's destination position
	if (colSpace1.PremoveCollisionCheckWithSlope(Position(), colSpace2, other._predictedPosition))
		return true;

	return false;
}

//==============================================================================


#if SW_DBSTREAM >= 1

std::ostream&
operator << ( std::ostream& s, const PhysicalAttributes& box )
{
	s << "Physical:" << std::endl;
	s << "  Position:           " << box.Position() << std::endl;
	s << "  _predictedPosition: " << box._predictedPosition << std::endl;
	s << "  _colSpace: ";
	Print(s,box._colSpace,box.Position());

	s << " old velocity : " << box.OldLinVelocity() << std::endl;
	s << "  linVelocity : " << box.linVelocity << std::endl;
	s << " meanVelocity : " << box.MeanVelocity() << std::endl;
	return s;
}

#endif // SW_DBSTREAM >= 1

//==============================================================================
                      

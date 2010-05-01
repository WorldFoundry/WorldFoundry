//==============================================================================
// plane.cc:
//==============================================================================
// Copyright (c) 1997,1999,2000 World Foundry Group, 
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
//
// Description: The Plane class encapsulates a planar sloped surface.
//				The data is stored as coefficients to the plane equation:
//					Ax + By + Cz + D = 0
//
// Original Author: Phil Torre, 7/17/97 14:24
//
//==============================================================================

#include <math/plane.hp>

//==============================================================================

Plane::Plane()
	: _normal(Vector3::zero), _offset(Scalar::zero)
{

}

//==============================================================================

Plane::Plane(Vector3& normal, Scalar& offset)
	: _normal(normal), _offset(offset)
{

}

//==============================================================================

Plane::Plane(Scalar& A, Scalar& B, Scalar& C, Scalar& D)
	: _normal(Vector3(A, B, C)), _offset(D)
{

}

//==============================================================================

#if SW_DBSTREAM >= 1

std::ostream&
operator << ( std::ostream& s, const Plane& plane )
{
	s << "A(" << plane.A() << "), B(" << plane.B() << "), C(" << plane.C() << "), D(" << plane.D() << ")" << std::endl;
	return s;
}

#endif // SW_DBSTREAM >= 1

//==============================================================================


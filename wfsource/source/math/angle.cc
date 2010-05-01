//==============================================================================
// angle.cc:
// Copyright (c) 1996,1997,1998,1999,2000,2002 World Foundry Group  
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
// Original Author: Kevin T. Seghetti
//==============================================================================

#define _ANGLE_CC

#include <math/angle.hp>
#include <math/scalar.hp>

// ------------------------------------------------------------------------
// static data members
// ------------------------------------------------------------------------

const Angle Angle::zero( Revolution( Scalar::zero ) );
const Angle Angle::one( Revolution( Scalar::one ) );
const Angle Angle::onequarter( Revolution( SCALAR_CONSTANT( 0.25 ) ) );
const Angle Angle::half( Revolution( Scalar::half ) );
const Angle Angle::threequarters( Revolution( SCALAR_CONSTANT( 0.75 ) ) );

// ------------------------------------------------------------------------
// IOStream Support
// ------------------------------------------------------------------------

#if DO_IOSTREAMS

std::ostream&
operator << ( std::ostream& s, const Angle& q )
{
	return s << std::dec << q.AsDegree() << " deg";
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------
// Binary I/O Stream Support
// ------------------------------------------------------------------------

binistream&
operator >> ( binistream& binis, Angle& angle )
{
	return binis >> angle._value;
}

// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const Angle angle )
{
	return binos << angle._value;
}

#endif

// ------------------------------------------------------------------------

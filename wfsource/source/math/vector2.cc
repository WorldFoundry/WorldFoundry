// ------------------------------------------------------------------------
// Copyright( c ) 1997,1998,1999,2000,2002 World Foundry Group  
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

// ------------------------------------------------------------------------

#define _VECTOR2_CC

// ------------------------------------------------------------------------
// 3D Vector Class: Inlined Functions
// ------------------------------------------------------------------------

#include <math/vector2.hp>

// ------------------------------------------------------------------------

const Vector2 Vector2::zero = Vector2( Scalar::zero, Scalar::zero );
const Vector2 Vector2::one = Vector2(  Scalar::one, Scalar::one );
const Vector2 Vector2::unitX = Vector2( Scalar::one, Scalar::zero );
const Vector2 Vector2::unitY = Vector2( Scalar::zero, Scalar::one );
const Vector2 Vector2::unitNegativeX = Vector2( Scalar::negativeOne, Scalar::zero );
const Vector2 Vector2::unitNegativeY = Vector2( Scalar::zero, Scalar::negativeOne );

// ------------------------------------------------------------------------
// Debug ostream support

#if DO_IOSTREAMS

std::ostream&
operator << ( std::ostream& os, const Vector2& v )
{
        os << "( " << v._arrScalar[0] << ", " << v._arrScalar[1] << " )";
	return os;
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------
// Binary I/O Stream Support
// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const Vector2& v )
{
        return binos << v._arrScalar[0] << v._arrScalar[1];
}

#endif

// ------------------------------------------------------------------------

binistream&
operator >> ( binistream& binis, Vector2& v )
{
        return binis >> v._arrScalar[0] >> v._arrScalar[1];
}

//=============================================================================

Scalar
Vector2::Length() const
{
#if defined(SCALAR_TYPE_FIXED)
	long a = X().AsLong();
	long b = Y().AsLong();
   register unsigned long out0;
	register long out1;
	register unsigned long out2;
	register long out3;

	// result = sqrt(a*a + b*b)
//	MultiplyAndNop64(a,a,out1,out0);
	MultiplyAndNop64(a,a,out1,out0);
	Multiply64(b,b,out3,out2);
	AddCarry64(out1,out0,out3,out2);

	return Scalar(Sqrt64(out1,out0));
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   return ((X()*X())+(Y()*Y())).Abs().Sqrt();
#else
#error SCALAR TYPE not defined
#endif
}

//-----------------------------------------------------------------------------

Scalar
Vector2::RLength() const
{
#if defined(SCALAR_TYPE_FIXED)
	int32 a = X().AsLong();
	int32 b = Y().AsLong();
    register unsigned long out0;
	register long out1;
	register unsigned long out2;
	register long out3;

	// result = sqrt(a*a + b*b)
	MultiplyAndNop64(a,a,out1,out0);

	Multiply64(b,b,out3,out2);

	AddCarry64(out1,out0,out3,out2);

	return Scalar(FastRSqrt64(out1,out0));
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
   return ((X()*X())+(Y()*Y())).Abs().FastSqrt();
#else
#error SCALAR TYPE not defined
#endif
}

//=============================================================================

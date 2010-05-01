
#define _QUAT_CC

// ------------------------------------------------------------------------
// Copyright (c) 1997, PF. Magic, Inc.  
//
// This is UNPUBLISHED PROPRIETARY SOURCE CODE of PF. Magic, Inc.;
// The contents of this file may not be disclosed to third parties, copied
// or duplicated in any form, in whole or in part, without the prior
// written permission of PF. Magic, Inc.
// ------------------------------------------------------------------------

#include <math/quat.hp>
#include <math/scalar.hp>
#include <visual/brstrmi.hp>
#include <visual/veuler.hp>
#include <math/matrix34.hp>
#if defined( __PSX__ )
#	include <math/matrix3t.hp>
#endif

#if defined( WRITER )
#include <visual/brstrmo.hp>
#endif

// ------------------------------------------------------------------------
// Quaternion Class
// ------------------------------------------------------------------------

void
Quaternion::Construct
(
	const Scalar x, const Scalar y, const Scalar z, const Scalar w
)
{
	SetX( x );
	SetY( y );
	SetZ( z );
	SetW( w );
}

// ------------------------------------------------------------------------

void
Quaternion::ConstructIdentity()
{
	Construct( Scalar::zero, Scalar::zero, Scalar::zero, Scalar::zero );
}

// ------------------------------------------------------------------------

void
Quaternion::ConstructEuler( const Euler& euler )
{
	assert(0);				// kts let me know if you need this
//	XBrEulerToQuat( BrQuatPtr(), (br_euler *)euler.BrEulerPtr() );
		// const-cast
}

// ------------------------------------------------------------------------

void
Quaternion::ConstructMatrix34( const Matrix34& matrix34 )
{
	assert(0);				// kts let me know if you need this
//	XBrMatrix34ToQuat( BrQuatPtr(), (br_matrix34 *)matrix34.BrMatrix34Ptr() );
		// const-cast
}

// ------------------------------------------------------------------------

#if defined( __PSX__ )

void
Quaternion::ConstructMatrix3t( const Matrix3t& matrix3t )
{
	assert(0);				// kts let me know if you need this
	// brm: non-optimal, but BrMatrix3tToQuat does not exist
//	Matrix34 matrix34( matrix3t );
//	ConstructMatrix34( matrix34 );
}

#endif

// ------------------------------------------------------------------------

#if DO_IOSTREAMS

ostream&
operator << ( ostream& os, const Quaternion& x )
{
	return os << "( "
		<< x.GetX() << " "
		<< x.GetY() << " "
		<< x.GetZ() << " "
		<< x.GetW()
		<< " )";
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------

binistream&
operator >> ( binistream& binis, Quaternion& x )
{
	return binis >> x._quat;
}

// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const Quaternion& x )
{
	return binos << x._quat;
}

#endif // defined( WRITER )

// ------------------------------------------------------------------------


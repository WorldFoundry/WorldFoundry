// ------------------------------------------------------------------------
// Copyright (c) 1996,1997,1998,1999,2000 World Foundry Group  
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

#define _EULER_CC

#include <math/euler.hp>
#include <math/vector2.hp>
#include <math/vector3.hp>

#if defined( WRITER )
#	include <visual/brstrmo.hp>
#endif

// ------------------------------------------------------------------------
// static data members
// ------------------------------------------------------------------------

const Euler Euler::zero = Euler(
	Angle::Revolution(SCALAR_CONSTANT(0)),
	Angle::Revolution(SCALAR_CONSTANT(0)),
	Angle::Revolution(SCALAR_CONSTANT(0))
);

// ------------------------------------------------------------------------
// Euler Angle Class
// ------------------------------------------------------------------------

Euler::Euler
(
	const Angle a, const Angle b, const Angle c
)
{
	Construct( a, b, c );
}

// ------------------------------------------------------------------------

Euler::Euler()
{
	ConstructIdentity();
}

// ------------------------------------------------------------------------

//Euler::Euler( const Matrix34& matrix34 )
//{
//	ConstructMatrix34( matrix34 );
//}

// ------------------------------------------------------------------------

Euler::Euler( binistream& binis )
{
	Read( binis );
	ValidateObject( *this );
}

// ------------------------------------------------------------------------

void
Euler::SetLookAt( const Vector3& lookAt )
{
	Vector3 v = lookAt;
//	cscreen << "SLA: " << v << endl;
	Angle theta( v.Y().ATan2( v.X() ) );
//	cscreen << dec << "theta = " << theta << endl;
	SetC( theta );
	v.RotateZ( theta );
//	cscreen << "rot: " << v << endl;
	Angle phi( v.Y().ATan2( v.Z() ));
	// - Angle::Revolution( SCALAR_CONSTANT( 0.25 ) ) );
//	cscreen << "phi = " << phi << endl;
	SetB( phi );
	SetA( Angle::zero );
//	cscreen << "Result: " << *this << endl;
}

// ------------------------------------------------------------------------

void
Euler::ConstructIdentity()
{
	Construct( Angle::zero, Angle::zero, Angle::zero );
}

// ------------------------------------------------------------------------

//void
//Euler::ConstructMatrix34( const Matrix34& mat )
//{
//	Scalar cy = Vector2(mat[0][0],mat[0][1]).Length();
//
//	if(cy > SCALAR_CONSTANT(long(0x0010)) )
//	{
//		_a = mat[1][2].ATan2(mat[2][2]);
//		_b = (-mat[0][2]).ATan2(cy);
//		_c =  mat[0][1].ATan2(mat[0][0]);
//	}
//	else
//	{
//		_a = (-mat[2][1]).ATan2(mat[1][1]);
//		_b = (-mat[0][2]).ATan2(cy);
//		_c = 0;
//	}
//}

// ------------------------------------------------------------------------
// Binary I/O Stream Support
// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const Euler& x )
{
	// simple error checking
	AssertMsg( !binos.bad(), "Stream is not valid" );

	DBSTREAM4( cbrstrm << "operator << ( binostream& " << (void *)&binos
		<< ", Euler3& " << (void *)&e << " )" << endl; )

	binos.align( binos.alignobject() );
	binios::streampos data_start = binos.tellp();

	return binos << x._a << x._b << x._c;
}

#endif	// WRITER

// ------------------------------------------------------------------------
// debug output stream support
// ------------------------------------------------------------------------

#if DO_IOSTREAMS

// ------------------------------------------------------------------------

std::ostream&
operator << ( std::ostream& os , const Euler& x )
{
	return os << "( "
		<< x.GetA() << ", "
		<< x.GetB() << ", "
		<< x.GetC() << " )";
}

// ------------------------------------------------------------------------
#endif // DO_IOSTREAMS
// ------------------------------------------------------------------------

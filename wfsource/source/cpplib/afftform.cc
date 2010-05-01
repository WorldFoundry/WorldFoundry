
#define _AFFTFORM_CC

// ------------------------------------------------------------------------
// Copyright (c) 1997-1999, World Foundry Group  
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

#include <cpplib/afftform.hp>
#include <cpplib/vector3.hp>
//#include <cpplib/quat.hp>
#include <cpplib/matrix34.hp>
#include <cpplib/euler.hp>
#include <visual/binstrm.hp>
#if defined( __PSX__ )
#	include <cpplib/matrix3t.hp>
#endif

// ------------------------------------------------------------------------
// Affine Transform Class
// ------------------------------------------------------------------------

AffineTransform::AffineTransform()
{
	ConstructIdentity();
}

// ------------------------------------------------------------------------

AffineTransform::AffineTransform( const Vector3& translation )
{
	ConstructTranslation( translation );
}

// ------------------------------------------------------------------------

AffineTransform::AffineTransform( const Euler& euler, const Vector3& translation )
{
	ConstructEuler( euler, translation );
}

// ------------------------------------------------------------------------

//AffineTransform::AffineTransform( const Quaternion& quaternion, const Vector3& translation )
//{
//	ConstructQuaternion( quaternion, translation );
//}

// ------------------------------------------------------------------------

AffineTransform::AffineTransform( const Matrix34& matrix34 )
{
	ConstructMatrix34( matrix34 );
}

// ------------------------------------------------------------------------

#if defined( __PSX__ )

AffineTransform::AffineTransform( const Matrix3t& matrix3t )
{
	ConstructMatrix3t( matrix3t );
}

#endif

// ------------------------------------------------------------------------
// type query/set
// ------------------------------------------------------------------------

AffineTransform::Type
AffineTransform::GetType() const
{
//	ValidateObject( *this );					// kts 3/20/97 14:09: ValidateObject calls GetType
	return AffineTransform::Type( _t.type );
}

// ------------------------------------------------------------------------

void
AffineTransform::SetType( AffineTransform::Type type )
{
	_t.type = uint16( type );
	ValidateObject( *this );
}

// ------------------------------------------------------------------------
// initialization methods
// ------------------------------------------------------------------------

void
AffineTransform::ConstructIdentity()
{
	SetType( AffineTransform::IDENTITY );
	ValidateObject( *this );
}

// ------------------------------------------------------------------------

void
AffineTransform::ConstructTranslation( const Vector3& translation )
{
	SetType( AffineTransform::TRANSLATION );
	AsTranslate() = translation;
	ValidateObject( *this );
}

// ------------------------------------------------------------------------

void
AffineTransform::ConstructEuler( const Euler& euler, const Vector3& translation )
{
	SetType( AffineTransform::EULER );
	AsEuler() = euler;
	AsEulerTranslate() = translation;
	ValidateObject( *this );
}

// ------------------------------------------------------------------------

//void
//AffineTransform::ConstructQuaternion( const Quaternion& quaternion, const Vector3& translation )
//{
//	SetType( AffineTransform::QUATERNION );
//	AsQuaternion() = quaternion;
//	AsQuaternionTranslate() = translation;
//	ValidateObject( *this );
//}

// ------------------------------------------------------------------------

void
AffineTransform::ConstructMatrix34( const Matrix34& matrix34 )
{
	SetType( AffineTransform::MATRIX34 );
	AsMatrix34() = matrix34;
	ValidateObject( *this );
}

// ------------------------------------------------------------------------

#if defined( __PSX__ )

void
AffineTransform::ConstructMatrix3t( const Matrix3t& matrix3t )
{
	SetType( AffineTransform::MATRIX3T );
	AsMatrix3t() = matrix3t;
	ValidateObject( *this );
}

#endif

// ------------------------------------------------------------------------
// conversion methods
// ------------------------------------------------------------------------

void
AffineTransform::GetEuler( Euler& euler ) const
{
	switch( GetType() )
	{
		case AffineTransform::IDENTITY:
		case AffineTransform::TRANSLATION:
			euler.SetA( Angle::zero );
			euler.SetB( Angle::zero );
			euler.SetC( Angle::zero );
		break;

		case AffineTransform::EULER:
			euler = AsEuler();
		break;

//		case AffineTransform::QUATERNION:
//			euler.ConstructQuaternion( AsQuaternion() );
//		break;

//		case AffineTransform::MATRIX34:
//			euler.ConstructMatrix34( AsMatrix34() );
//		break;

//#if defined( __PSX__ )
//		case AffineTransform::MATRIX3T:
//			euler.ConstructMatrix3t( AsMatrix3t() );
//		break;
//#endif

		default:
			Fail( "Not yet implemented" );
	}
}

// ------------------------------------------------------------------------

//void
//AffineTransform::GetQuaternion( Quaternion& quaternion ) const
//{
//	switch( GetType() )
//	{
//		case AffineTransform::IDENTITY:
//		case AffineTransform::TRANSLATION:
//			quaternion.ConstructIdentity();
//		break;

//		case AffineTransform::EULER:
//			quaternion.ConstructEuler( AsEuler() );
//		break;

//		case AffineTransform::QUATERNION:
//			quaternion = AsQuaternion();
//		break;

//		case AffineTransform::MATRIX34:
//			quaternion.ConstructMatrix34( AsMatrix34() );
//		break;

//#if defined( __PSX__ )
//		case AffineTransform::MATRIX3T:
//			quaternion.ConstructMatrix3t( AsMatrix3t() );
//		break;
//#endif

//		default:
//			Fail( "Not yet implemented" );
//	}
//}

// ------------------------------------------------------------------------

void
AffineTransform::GetMatrix34( Matrix34& matrix34 ) const
{
	switch( GetType() )
	{
		case AffineTransform::IDENTITY:
			matrix34.ConstructIdentity();
		break;

		case AffineTransform::TRANSLATION:
			matrix34.ConstructTranslation( GetTranslation() );
		break;

		case AffineTransform::EULER:
			matrix34.ConstructEuler( AsEuler(), GetTranslation() );
		break;

//		case AffineTransform::QUATERNION:
//			matrix34.ConstructQuaternion( AsQuaternion(), GetTranslation() );
//		break;

		case AffineTransform::MATRIX34:
			matrix34 = AsMatrix34();
		break;

//#if defined( __PSX__ )
//		case AffineTransform::MATRIX3T:
//			matrix34.ConstructMatrix3t( AsMatrix3t() );
//		break;
//#endif

		default:
			Fail( "Not yet implemented" );
	}
}

// ------------------------------------------------------------------------

#if defined( __PSX__ )

void
AffineTransform::GetMatrix3t( Matrix3t& matrix3t ) const
{
	switch( GetType() )
	{
		case AffineTransform::IDENTITY:
			matrix3t.ConstructIdentity();
		break;

		case AffineTransform::TRANSLATION:
			matrix3t.ConstructTranslation( GetTranslation() );
		break;

//		case AffineTransform::EULER:
//			matrix3t.ConstructEuler( AsEuler(), GetTranslation() );
//		break;

//		case AffineTransform::QUATERNION:
//			matrix3t.ConstructQuaternion( AsQuaternion(), GetTranslation() );
//		break;

//		case AffineTransform::MATRIX34:
//			matrix3t.ConstructMatrix34( AsMatrix34() );
//		break;

		case AffineTransform::MATRIX3T:
			matrix3t = AsMatrix3t();
		break;

		default:
			Fail( "Not yet implemented" );
	}
}

#endif

// ------------------------------------------------------------------------
// query/set translation
// ------------------------------------------------------------------------

void
AffineTransform::SetTranslation( const Vector3& translation )
{
	switch( GetType() )
	{
		case AffineTransform::IDENTITY:
			Fail( "Can't call AffineTransform::SetTranslation on an identity transform" );
		break;

		case AffineTransform::TRANSLATION:
			AsTranslate() = translation;
		break;

		case AffineTransform::EULER:
			AsEulerTranslate() = translation;
		break;

//		case AffineTransform::QUATERNION:
//			AsQuaternionTranslate() = translation;
//		break;

		case AffineTransform::MATRIX34:
			AsMatrix34()[3] = translation;
		break;

#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
			AsMatrix3t().SetTranslation( translation );
		break;
#endif

		default:
			Fail( "Not yet implemented" );
	}
}

// ------------------------------------------------------------------------
// will only change the type if the type is IDENTITY

Vector3
AffineTransform::GetTranslation() const
{
	switch( GetType() )
	{
		case AffineTransform::IDENTITY:
			return Vector3( Scalar::zero, Scalar::zero, Scalar::zero );

		case AffineTransform::TRANSLATION:
			return AsTranslate();

		case AffineTransform::EULER:
			return AsEulerTranslate();

//		case AffineTransform::QUATERNION:
//			return AsQuaternionTranslate();

		case AffineTransform::MATRIX34:
			return AsMatrix34()[3];

#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
			return AsMatrix3t().GetTranslation();
#endif

		default:
			Fail( "Not yet implemented" );
	}

	return Vector3::zero;
}

// ------------------------------------------------------------------------
// operators
// ------------------------------------------------------------------------

AffineTransform
AffineTransform::operator * ( const AffineTransform& x ) const
{
	// create a new 3 x 4 matrix transform, copy this transform into it
	AffineTransform result( AsMatrix34() );
	switch( x.GetType() )
	{
		case AffineTransform::IDENTITY:
			// no effect/just a copy construction
		break;

		case AffineTransform::TRANSLATION:
			result.SetTranslation( result.GetTranslation() + x.GetTranslation() );
		break;

		case AffineTransform::EULER:
//		case AffineTransform::QUATERNION:
		{
			Matrix34 xMatrix;
			x.GetMatrix34( xMatrix );
			result.AsMatrix34() *= xMatrix;
		}
		break;

		case AffineTransform::MATRIX34:
			result.AsMatrix34() *= x.AsMatrix34();
		break;

#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
		{
			Matrix34 rhs;
			x.GetMatrix34( rhs );
			result.AsMatrix34() *= rhs;
		}
		break;
#endif

		default:
			Fail( "Not yet implemented" );
	}
	return result;
}

// ------------------------------------------------------------------------

const AffineTransform&
AffineTransform::operator *= ( const AffineTransform& x )
{
	(*this) = (*this) * x;	// operator *, then copy constructor
	return *this;
}

// ------------------------------------------------------------------------
// helpers functions
// ------------------------------------------------------------------------

const Vector3&
AffineTransform::AsTranslate() const
{
	return *( (const Vector3 *)&_t.t.translate.t );
}

// ------------------------------------------------------------------------

//const Vector3&
//AffineTransform::AsQuaternionTranslate() const
//{
//	return *( (const Vector3 *)&_t.t.quat.t );
//}

// ------------------------------------------------------------------------

const Vector3&
AffineTransform::AsEulerTranslate() const
{
	return *( (const Vector3 *)&_t.t.euler.t );
}

// ------------------------------------------------------------------------

const Euler&
AffineTransform::AsEuler() const
{
	return *( (const Euler *)&_t.t.euler.e );
}

// ------------------------------------------------------------------------

//const Quaternion&
//AffineTransform::AsQuaternion() const
//{
//	return *( (const Quaternion *)&_t.t.quat.q );
//}

// ------------------------------------------------------------------------

const Matrix34&
AffineTransform::AsMatrix34() const
{
	return *( (const Matrix34 *)&_t.t.mat );
}

// ------------------------------------------------------------------------

#if defined( __PSX__ )

const Matrix3t&
AffineTransform::AsMatrix3t() const
{
	return *( (const Matrix3t *)&_t.t.mat3t.mat );
}

#endif

// ------------------------------------------------------------------------

Vector3&
AffineTransform::AsTranslate()
{
	return *( (Vector3 *)&_t.t.translate.t );
}

// ------------------------------------------------------------------------

//Vector3&
//AffineTransform::AsQuaternionTranslate()
//{
//	return *( (Vector3 *)&_t.t.quat.t );
//}

// ------------------------------------------------------------------------

Vector3&
AffineTransform::AsEulerTranslate()
{
	return *( (Vector3 *)&_t.t.euler.t );
}

// ------------------------------------------------------------------------

Euler&
AffineTransform::AsEuler()
{
	return *( (Euler *)&_t.t.euler.e );
}

// ------------------------------------------------------------------------

//Quaternion&
//AffineTransform::AsQuaternion()
//{
//	return *( (Quaternion *)&_t.t.quat.q );
//}

// ------------------------------------------------------------------------

Matrix34&
AffineTransform::AsMatrix34()
{
	return *( (Matrix34 *)&_t.t.mat );
}

// ------------------------------------------------------------------------

#if defined( __PSX__ )

Matrix3t&
AffineTransform::AsMatrix3t()
{
	return *( (Matrix3t *)&_t.t.mat3t.mat );
}

#endif

// ------------------------------------------------------------------------
// ostream support
// ------------------------------------------------------------------------

#if DO_IOSTREAMS

ostream&
operator << ( ostream& os, const AffineTransform& x )
{
	switch( x.GetType() )
	{
		case AffineTransform::IDENTITY:
			os << "IDENTITY";
		break;

		case AffineTransform::TRANSLATION:
			os << "TRANSLATION " << x.GetTranslation();
		break;

		case AffineTransform::EULER:
			os << "EULER " << x.AsEuler();
		break;

//		case AffineTransform::QUATERNION:
//			os << "QUATERNION " << x.AsQuaternion();
//		break;

		case AffineTransform::MATRIX34:
		{
			os << "MATRIX34";
			os << "( ";
			for( int idxRow = 0; idxRow < 4; ++idxRow )
			{
				os << "( ";
				for( int idxColumn = 0; idxColumn < 3; ++idxColumn )
				{
					os << x.AsMatrix34()[idxRow][idxColumn] << " ";
				}
				os << ") ";
			}
			os << ")";
		}
		break;

#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
		{
			os << "MATRIX3T";
			os << "( ";
			for( int idxRow = 0; idxRow < 4; ++idxRow )
			{
				os << "( ";
				for( int idxColumn = 0; idxColumn < 3; ++idxColumn )
				{
					os << x.AsMatrix3t().GetElement( idxRow, idxColumn ) << " ";
				}
				os << ") ";
			}
			os << ")";
		}
		break;
#endif

		default:
			Fail( "Not yet implemented" );
	}

	return os;
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------
// binary io stream support
// ------------------------------------------------------------------------

binistream&
operator >> ( binistream& binis, AffineTransform& x )
{
	binis >> (uint16&)x._t.type;
	switch( x.GetType() )
	{
		case AffineTransform::IDENTITY:
		break;

		case AffineTransform::TRANSLATION:
			binis >> x.AsTranslate();
		break;

		case AffineTransform::EULER:
			binis >> x.AsEuler() >> x.AsEulerTranslate();
		break;

//		case AffineTransform::QUATERNION:
//			binis >> x.AsQuaternion() >> x.AsQuaternionTranslate();
//		break;

		case AffineTransform::MATRIX34:
			binis >> x.AsMatrix34();
		break;

#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
			binis >> x.AsMatrix3t();
		break;
#endif

		default:
			Fail( "Not yet implemented" );
	}

	return binis;
}

// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const AffineTransform& x )
{
	binos << uint16( x.GetType() );
	switch( x.GetType() )
	{
		case AffineTransform::IDENTITY:
		break;

		case AffineTransform::TRANSLATION:
			binos << x.AsTranslate();
		break;

		case AffineTransform::EULER:
			binos << x.AsEuler() << x.AsEulerTranslate();
		break;

//		case AffineTransform::QUATERNION:
//			binos << x.AsQuaternion() << x.AsQuaternionTranslate();
//		break;

		case AffineTransform::MATRIX34:
			binos << x.AsMatrix34();
		break;

#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
			binos << x.AsMatrix3t();
		break;
#endif

		default:
			Fail( "Not yet implemented" );
	}

	return binos;
}

#endif /* defined( WRITER ) */

// ------------------------------------------------------------------------
// validation
// ------------------------------------------------------------------------

#if DEBUG

void
AffineTransform::Validate( const char * file, const int line ) const
{
	switch( GetType() )
	{
		case AffineTransform::IDENTITY:
		case AffineTransform::TRANSLATION:
		case AffineTransform::EULER:
//		case AffineTransform::QUATERNION:
		case AffineTransform::MATRIX34:
#if defined( __PSX__ )
		case AffineTransform::MATRIX3T:
#endif
		break;


		default:
			Fail( "AffineTransform failed validation.  Invalid type : " << GetType() );
	}
}

#endif /* DEBUG */

// ------------------------------------------------------------------------



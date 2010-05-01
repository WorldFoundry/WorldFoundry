// ------------------------------------------------------------------------
// math/matrix3t.cc:
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


#define _MATRIX3T_CC


#include <math/matrix3t.hp>
#include <math/vector3.hp>
#include <math/matrix34.hp>


#if defined( __PSX__ )
#include <libgte.h>
#include <inline_c.h>
#include <gtemac.h>
#endif

extern MachineType gTargetMachineType;


//=============================================================================
// kts added 8/5/97 19:46

#if defined( __PSX__ )

void
Matrix3t::SetGTE()
{
	MATRIX mat3t;
	gte_SetRotMatrix(&_matrix);
	mat3t.t[0] = _matrix.t[0].AsLong() >> 8;
	mat3t.t[1] = _matrix.t[1].AsLong() >> 8;
	mat3t.t[2] = _matrix.t[2].AsLong() >> 8;
	gte_SetTransMatrix(&mat3t);
}

#endif

#if 0
// ------------------------------------------------------------------------
// 3 x 4 Matrix Class
// ------------------------------------------------------------------------

Scalar
Matrix3t::GetElement( int idxRow, int idxColumn ) const
{
	assert( idxRow >= 0 );
	assert( idxRow < 4 );
	assert( idxColumn >= 0 );
	assert( idxColumn < 3 );

	if( idxRow <= 2 )
	{	// 3 x 3 matrix component
		return Scalar( long( _matrix.m[idxRow][idxColumn] ) << 4 );
	}
	else
	{	// translation component
		return Scalar( _matrix.t[idxColumn] );
	}
}

// ------------------------------------------------------------------------

void
Matrix3t::SetElement( int idxRow, int idxColumn, const Scalar& value )
{
	assert( idxRow >= 0 );
	assert( idxRow < 4 );
	assert( idxColumn >= 0 );
	assert( idxColumn < 3 );

	if( idxRow <= 2 )
	{	// 3 x 3 matrix component
		_matrix.m[idxRow][idxColumn] = value.AsGTEScalar();
	}
	else
	{	// translation component
		_matrix.t[idxColumn] = value;
	}
}

// ------------------------------------------------------------------------

void
Matrix3t::Construct
(
	const Vector3& row0,
	const Vector3& row1,
	const Vector3& row2,
	const Vector3& row3
)
{
	SetRow( 0, row0 );
	SetRow( 1, row1 );
	SetRow( 2, row2 );
	SetTranslation( row3 );
}

// ------------------------------------------------------------------------

void
Matrix3t::ConstructTranslation( const Vector3& translation )
{
	SetRow( 0, Vector3( Scalar::one, Scalar::zero, Scalar::zero ) );
	SetRow( 1, Vector3( Scalar::zero, Scalar::one, Scalar::zero ) );
	SetRow( 2, Vector3( Scalar::zero, Scalar::zero, Scalar::one ) );
	SetTranslation( translation );
}

// ------------------------------------------------------------------------

//void
//Matrix3t::ConstructScale( const Vector3& s, const Vector3& translation )
//{
//#if defined( __PSX__ )
//	XBrMatrix3tScale( BrMatrix3tPtr(),
//		s.X().AsLong(), s.Y().AsLong(), s.Z().AsLong() );
//#else
//	Fail( "Only implemented on PSX" );
//#endif
//	SetTranslation( translation );
//}

// ------------------------------------------------------------------------

//void
//Matrix3t::ConstructEuler( const Euler& euler, const Vector3& translation )
//{
//	// brm: non-optimal, but BrEulerToMatrix3t does not exist
//	Matrix34 matrix34( euler, translation );
//	ConstructMatrix34( matrix34 );
//}

// ------------------------------------------------------------------------

#endif

void
Matrix3t::ConstructMatrix34( const Matrix34& matrix34 )
{
//	XBrMatrix3tCopy34( BrMatrix3tPtr(), matrix34.BrMatrix34Ptr() );


// kts for now do the swap in SetGTE
#if 0
	_matrix.m[0][0] = matrix34[0][0].AsGTEScalar();
	_matrix.m[1][0] = matrix34[0][1].AsGTEScalar();
	_matrix.m[2][0] = matrix34[0][2].AsGTEScalar();

	_matrix.m[0][1] = matrix34[1][0].AsGTEScalar();
	_matrix.m[1][1] = matrix34[1][1].AsGTEScalar();
	_matrix.m[2][1] = matrix34[1][2].AsGTEScalar();

	_matrix.m[0][2] = matrix34[2][0].AsGTEScalar();
	_matrix.m[1][2] = matrix34[2][1].AsGTEScalar();
	_matrix.m[2][2] = matrix34[2][2].AsGTEScalar();

	_matrix.t[0] = matrix34[3][0].AsLong() >> 8;
	_matrix.t[1] = matrix34[3][1].AsLong() >> 8;
	_matrix.t[2] = matrix34[3][2].AsLong() >> 8;
#else
	_matrix.m[0][0] = matrix34[0][0].AsGTEScalar();
	_matrix.m[0][1] = matrix34[0][1].AsGTEScalar();
	_matrix.m[0][2] = matrix34[0][2].AsGTEScalar();

	_matrix.m[1][0] = matrix34[1][0].AsGTEScalar();
	_matrix.m[1][1] = matrix34[1][1].AsGTEScalar();
	_matrix.m[1][2] = matrix34[1][2].AsGTEScalar();

	_matrix.m[2][0] = matrix34[2][0].AsGTEScalar();
	_matrix.m[2][1] = matrix34[2][1].AsGTEScalar();
	_matrix.m[2][2] = matrix34[2][2].AsGTEScalar();

	_matrix.t[0] = matrix34[3][0];
	_matrix.t[1] = matrix34[3][1];
	_matrix.t[2] = matrix34[3][2];
#endif
}

// ------------------------------------------------------------------------

#if 0
Matrix3t
Matrix3t::operator * ( const Matrix3t& rhs ) const
{
	return Matrix3t( *this ) *= rhs;
}

// ------------------------------------------------------------------------


const Matrix3t&
Matrix3t::operator *= ( const Matrix3t& rhs )
{
#if defined(__PSX__)
	wf_matrix3t mattmp2;

	Vector3 v;
//	SetRotMatrix((MATRIX *)&_matrix);
	gte_SetRotMatrix((MATRIX *)&_matrix);
	MulRotMatrix0((MATRIX *)&rhs._matrix, (MATRIX *)&mattmp2);
	ApplyTransposeMatrixLV((MATRIX *)&rhs._matrix, (VECTOR *)&_matrix.t, &v);

	mattmp2.t = rhs._matrix.t + v;
	_matrix = mattmp2;
#else
	Fail( "Only implemented on PSX" );
#endif
	return *this;
}

// ------------------------------------------------------------------------

void
Matrix3t::SetRow( int idxRow, const Vector3& x )
{
	assert( idxRow >= 0 );
	assert( idxRow < 4 );
	if(idxRow == 3)
	{
		_matrix.t = x;
	}
	else
	{
		_matrix.m[idxRow][0] = x[0].AsGTEScalar();
		_matrix.m[idxRow][1] = x[1].AsGTEScalar();
		_matrix.m[idxRow][2] = x[2].AsGTEScalar();
	}
}

// ------------------------------------------------------------------------
#endif

#if DO_IOSTREAMS

//ostream&
//operator << ( ostream& os, const Matrix3t& x )
//{
//	os << "(";
//	for( int idxRow = 0; idxRow < 4; ++idxRow )
//	{
//		os << " (";
//		for( int idxColumn = 0; idxColumn < 3; ++idxColumn )
//		{
//	//			os << " " << x.GetElement( idxRow, idxColumn ) << endl;
//			assert(0);				// kts write this
//		}
//		os << " )";
//	}
//	return os << " )";
//}

ostream&
operator << ( ostream& os, const Matrix3t& x )
{
//	return os << x._matrix;
	os << "Matrix3t " << (void *)&x << endl;

	for( int row = 0; row < 3; row++ )
	{
		for( int col = 0; col < 3; col++ )
		{
			os << Scalar( int32( x._matrix.m[row][col] ) << 4  ) << " ";
		}

		os << endl;
	}

	for( int col = 0; col < 3; col++ )
	{
		os << x._matrix.t[col] << " ";
	}
	os << endl;

	return os;
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------

#if 0
binistream&
operator >> ( binistream& binis, Matrix3t& x )
{
//	return binis >> x._matrix;

	DBSTREAM4( cbrstrm << "operator >> ( binistream& " << (void *)&binis
		<< ", Matrix3t& " << (void *)&x << " )" << endl; )

	binis.align( binis.alignobject() );
	binios::streampos data_start = binis.tellg();

	for( int row = 0; row < 3; ++row )
		for( int column = 0; column < 3; ++column )
			binis >> x._matrix.m[row][column];

	for( int column = 0; column < 3; ++column )
		binis >> x._matrix.t[column];

	AssertMsg( binis.tellg() - data_start == sizeof( Matrix3t ),
		"Size of Matrix3t read != sizeof( Matrix3t )" );

	return binis;
}

// ------------------------------------------------------------------------

#if defined( WRITER )

binostream&
operator << ( binostream& binos, const Matrix3t& x )
{
//	return binos << x._matrix;
	DBSTREAM4( cbrstrm << "operator << ( binostream& " << (void *)&binos
		<< ", wf_matrix3t& " << (void *)&x << " )" << endl; )

	binos.align( binos.alignobject() );
	binios::streampos data_start = binos.tellp();

	for( int row = 0; row < 3; ++row )
		for( int column = 0; column < 3; ++column )
			binos << x._matrix.m[row][column];

	for( int column = 0; column < 3; ++column )
		binos << x._matrix.t[column];

	if( gTargetMachineType == gHostMachineType )
	{
		AssertMsg( binos.tellp() - data_start == sizeof( Matrix3t ),
			"Size of Matrix3t write != sizeof( Matrix3t )" );
	}
	return binos;
}

#endif // defined( WRITER )

#endif
//=============================================================================

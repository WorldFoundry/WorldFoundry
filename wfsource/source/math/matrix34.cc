// ------------------------------------------------------------------------
// matrix34.cc
// Copyright (c) 1997,1998,1999,2000,2002 World Foundry Group  
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

#define _MATRIX34_CC

//-----------------------------------------------------------------------------

#include <math/matrix34.hp>
#include <math/vector2.hp>
#include <math/vector3.hp>
#include <math/euler.hp>

// ------------------------------------------------------------------------
// 3 x 4 Matrix Class
// ------------------------------------------------------------------------

void
Matrix34::Construct
(
	const Vector3& row0,
	const Vector3& row1,
	const Vector3& row2,
	const Vector3& row3
)
{
	(*this)[0] = row0;
	(*this)[1] = row1;
	(*this)[2] = row2;
	(*this)[3] = row3;
}

// ------------------------------------------------------------------------

void
Matrix34::ConstructTranslation( const Vector3& translation )
{
	(*this)[0] = Vector3( Scalar::one, Scalar::zero, Scalar::zero );
	(*this)[1] = Vector3( Scalar::zero, Scalar::one, Scalar::zero );
	(*this)[2] = Vector3( Scalar::zero, Scalar::zero, Scalar::one );
	(*this)[3] = translation;
}

// ------------------------------------------------------------------------

//void
//Matrix34::ConstructScale( const Vector3& s, const Vector3& translation )
//{
//	XBrMatrix34Scale( BrMatrix34Ptr(),
//		s.X().AsLong(), s.Y().AsLong(), s.Z().AsLong() );
//	(*this)[3] = translation;
//}

// ------------------------------------------------------------------------

void
Matrix34::ConstructEuler( const Euler& euler, const Vector3& translation )
{
	Scalar rotA(int16(0),euler.GetA().AsUInt16());
	Scalar rotB(int16(0),euler.GetB().AsUInt16());
	Scalar rotC(int16(0),euler.GetC().AsUInt16());

	Scalar sinA = rotA.Sin();
	Scalar sinB = rotB.Sin();
	Scalar sinC = rotC.Sin();

	Scalar cosA = rotA.Cos();
	Scalar cosB = rotB.Cos();
	Scalar cosC = rotC.Cos();

	Scalar cosAcosC = cosA * cosC;
	Scalar cosAsinC = cosA * sinC;

	Scalar sinAcosC = sinA * cosC;
	Scalar sinAsinC = sinA * sinC;

//	cscreen << euler << endl;
//	cscreen << "sin: " << si << "," << sj << "," << sinC << endl;
//	cscreen << "cos: " << cosA << "," << cosB << "," << cosC << endl;
//	cscreen << "cosAcosC: " << cosAcosC << ", cosAsinC: " << cosAsinC << endl;
//	cscreen << "sinAcosC: " << sinAcosC << ", sinAsinC: " << sinAsinC << endl;


//   a = bank
//   b = attitude
//   c = heading

	_matrix[0][0] = cosB * cosC;
	_matrix[0][1] = cosB * sinC;
	_matrix[0][2] = -sinB;
	_matrix[1][0] = (sinB * sinAcosC) - cosAsinC;
	_matrix[1][1] = (sinB*sinAsinC)+cosAcosC;
	_matrix[1][2] = cosB * sinA;
	_matrix[2][0] = (sinB*cosAcosC)+sinAsinC;
	_matrix[2][1] = (sinB*cosAsinC)-sinAcosC;
	_matrix[2][2] = cosB * cosA;

	_matrix[3][0] = _matrix[3][1] = _matrix[3][2] = Scalar::zero;
	(*this)[3] = translation;
}

//void
//Matrix34::ConstructEuler( const Euler& euler, const Vector3& translation )
//{
//	XBrEulerToMatrix34( (br_matrix34*)_matrix, (br_euler*)euler.BrEulerPtr() );
//	(*this)[3] = translation;
//}

//==============================================================================

//
//    br_scalar cy;
//    UASSERT(mat != NULL);
//    UASSERT(euler != NULL);
//    UASSERT(euler->order == BR_EULER_XYZ_S);
//
//    cy = BR_LENGTH2(M(0,0),M(0,1));
//
//    if(cy > BR_MUL(16,BR_SCALAR_EPSILON))
//    {
//       eX = BR_ATAN2( M(1,2), M(2,2));
//       eY = BR_ATAN2(-M(0,2),     cy);
//       eZ = BR_ATAN2( M(0,1), M(0,0));
//    }
//    else
//    {
//       eX = BR_ATAN2(-M(2,1), M(1,1));
//       eY = BR_ATAN2(-M(0,2),     cy);
//       eZ = 0;
//    }
//    return euler;


Euler
Matrix34::AsEuler() const
{
   Angle rotA, rotB, rotC;

#if 0
   Scalar length = Vector2(_matrix[0][0],_matrix[0][1]).Length();
   length.Validate();
   std::cout << "length of " << _matrix[0][0] << "," << _matrix[0][1] << " is " << length << std::endl;
   rotB = length.ATan2(-_matrix[0][2]);
   if(length > Scalar::epsilon)
   {
      rotA = _matrix[2][2].ATan2(_matrix[1][2]);
      rotC = _matrix[0][0].ATan2(_matrix[0][1]);
   }
   else
   {
      Scalar l2 = Vector2(_matrix[1][1],_matrix[2][1]);
      rotA = (-_matrix[1][1]).ATan2(_matrix[2][1]);
      rotC = Angle::zero;
   }

#else
   rotB = (-_matrix[0][2]).ASin();
   rotA = (_matrix[2][2]).ATan2(_matrix[1][2]);
   rotC = (_matrix[0][0]).ATan2(_matrix[0][1]);
#endif   
   rotA.Validate();
   rotB.Validate();
   rotC.Validate();
   return Euler(rotA, rotB, rotC);
}

// ------------------------------------------------------------------------

#if DO_IOSTREAMS

std::ostream&
operator << ( std::ostream& os, const Matrix34& x )
{
	os << std::endl;
	for( int idxRow = 0; idxRow < 4; ++idxRow )
	{
		os << x[idxRow] << std::endl;
	}
	return os;
}

#endif // DO_IOSTREAMS

// ------------------------------------------------------------------------

binistream&
operator >> ( binistream& binis, Matrix34& x )
{
//	return binis >> x._matrix;
	DBSTREAM4( cbrstrm << "operator >> ( binistream& " << (void *)&binis
		<< ", Matrix34& " << (void *)&x << " )" << endl; )

	binis.align( binis.alignobject() );
#if DO_ASSERTIONS
	binios::streampos data_start = binis.tellg();
#endif

	for( int row = 0; row < 4; row++ )
		for( int column = 0; column < 3; column++ )
//			binis >> BR_SCALAR_TO_QSCALAR_REF( _matrix[row][column] );
			binis >> x._matrix[row][column];

	AssertMsg( binis.tellg() - data_start == sizeof( Matrix34 ),
		"Size of Matrix34 read != sizeof( Matrix34 )" );

	return binis;
}

// ------------------------------------------------------------------------

#if defined( WRITER )

extern MachineType gTargetMachineType;

binostream&
operator << ( binostream& binos, const Matrix34& x )
{
//	return binos << x._matrix;
	DBSTREAM4( cbrstrm << "operator << ( binostream& " << (void *)&binos
		<< ", Matrix34& " << (void *)&x << " )" << endl; )

	binos.align( binos.alignobject() );
	binios::streampos data_start = binos.tellp();

	for( int row = 0; row < 4; row++ )
		for( int column = 0; column < 3; column++ )
			binos << x._matrix[row][column];

	if( gTargetMachineType == gHostMachineType )
	{
		AssertMsg( binos.tellp() - data_start == sizeof( Matrix34 ),
			"Size of Matrix34 write != sizeof( Matrix34 )" );
	}
	return binos;
}

#endif // defined( WRITER )


//=============================================================================

Vector3&
operator*= (Vector3& v,const Matrix34& matrix)
{
        Vector3 src(v);
        v[0] = (Mac3(src[0],matrix[0][0], src[1],matrix[1][0], src[2],matrix[2][0])+matrix[3][0]);
        v[1] = (Mac3(src[0],matrix[0][1], src[1],matrix[1][1], src[2],matrix[2][1])+matrix[3][1]);
        v[2] = (Mac3(src[0],matrix[0][2], src[1],matrix[1][2], src[2],matrix[2][2])+matrix[3][2]);
        return(v);
}

// ------------------------------------------------------------------------

Vector3&
MultiplyEqualAcross (Vector3& v,const Matrix34& matrix)
{
        Vector3 src(v);
        v[0] = (Mac3(src[0],matrix[0][0], src[1],matrix[0][1], src[2],matrix[0][2])+matrix[3][0]);
        v[1] = (Mac3(src[0],matrix[1][0], src[1],matrix[1][1], src[2],matrix[1][2])+matrix[3][1]);
        v[2] = (Mac3(src[0],matrix[2][0], src[1],matrix[2][1], src[2],matrix[2][2])+matrix[3][2]);
        return(v);
}

// ------------------------------------------------------------------------


// * From Graphics Gems II - Fast Matrix Inversion by Kevin Wu (pp. 342)
// *
// * Computes the inverse of a 3D affine matrix; i.e. a matrix with a dimen-
// * sionality of 4 where the right column has the entries (0, 0, 0, 1).
// *
// * This procedure treats the 4 by 4 matrix as a block matrix and
// * calculates the inverse of one submatrix for a significant perform-
// * ance improvement over a general procedure that can invert any non-
// * singular matrix:
// *          --        --          --          --
// *          |          | -1       |    -1      |
// *          | A      0 |          |   A      0 |
// *    -1    |          |          |            |
// *   M   =  |          |     =    |     -1     |
// *          | C      1 |          | -C A     1 |
// *          |          |          |            |
// *          --        --          --          --
// *
// *  where     M is a 4 by 4 matrix,
// *            A is the 3 by 3 ur left submatrix of M,
// *            C is the 1 by 3 lower left submatrix of M.
// *
// * Input:
// *   A   - 3D affine matrix
// *
// * Output:
// *   B  - inverse of 3D affine matrix
// *
// * Returned value:
// *   determinant of matrix

Scalar
Matrix34::Inverse(const Matrix34& source)
{
	assert(this != &source);			// cannot invert in place

    Scalar    idet,det;
    Scalar    pos, neg, temp;

#define ACCUMULATE (temp >= Scalar::zero) ? pos += temp : neg += temp;
#define PRECISION_LIMIT SCALAR_CONSTANT(1.0e-15)

	// Calculate the determinant of submatrix A and determine if the
	// the matrix is singular as limited by the double precision
	// floating-point data representation.

    pos = neg = Scalar::zero;
    temp =  (source[0][0] * source[1][1] * source[2][2]);
    ACCUMULATE
    temp =  (source[0][1] * source[1][2] * source[2][0]);
    ACCUMULATE
    temp =  (source[0][2] * source[1][0] * source[2][1]);
    ACCUMULATE
    temp = -(source[0][2] * source[1][1] * source[2][0]);
    ACCUMULATE
    temp = -(source[0][1] * source[1][0] * source[2][2]);
    ACCUMULATE
    temp = -(source[0][0] * source[1][2] * source[2][1]);
    ACCUMULATE
    det = pos + neg;
	// Is the submatrix A singular?
    if(det.Abs() <= Scalar::epsilon*SCALAR_CONSTANT(2))
		return Scalar::zero;

	if((det / (pos - neg)).Abs()  < PRECISION_LIMIT)
	{
		// Matrix M has no inverse
		return Scalar::zero;
	}

	// Calculate inverse(source) = adj(source) / det(source)
    idet = Scalar::one / det;

    (*this)[0][0] =  Mac2(source[1][1],source[2][2], -source[1][2],source[2][1]) * idet;
    (*this)[1][0] = -Mac2(source[1][0],source[2][2], -source[1][2],source[2][0]) * idet;
    (*this)[2][0] =  Mac2(source[1][0],source[2][1], -source[1][1],source[2][0]) * idet;
    (*this)[0][1] = -Mac2(source[0][1],source[2][2], -source[0][2],source[2][1]) * idet;
    (*this)[1][1] =  Mac2(source[0][0],source[2][2], -source[0][2],source[2][0]) * idet;
    (*this)[2][1] = -Mac2(source[0][0],source[2][1], -source[0][1],source[2][0]) * idet;
    (*this)[0][2] =  Mac2(source[0][1],source[1][2], -source[0][2],source[1][1]) * idet;
    (*this)[1][2] = -Mac2(source[0][0],source[1][2], -source[0][2],source[1][0]) * idet;
    (*this)[2][2] =  Mac2(source[0][0],source[1][1], -source[0][1],source[1][0]) * idet;

	// Calculate -C * inverse(source)
    (*this)[3][0] = -Mac3(source[3][0],(*this)[0][0], source[3][1],(*this)[1][0], source[3][2],(*this)[2][0]);
    (*this)[3][1] = -Mac3(source[3][0],(*this)[0][1], source[3][1],(*this)[1][1], source[3][2],(*this)[2][1]);
    (*this)[3][2] = -Mac3(source[3][0],(*this)[0][2], source[3][1],(*this)[1][2], source[3][2],(*this)[2][2]);

    return det;
}

//=============================================================================
// invert a matrix which is guaranteed to have a determinate of one
// (asserts this in debug mode)

void
Matrix34::InverseDetOne(const Matrix34& source)
{
#if DO_ASSERTIONS
    Scalar    pos, neg, temp;
#define ACCUMULATE (temp >= Scalar::zero) ? pos += temp : neg += temp;

#define EPSILON SCALAR_CONSTANT(.001)

	// Calculate the determinant of submatrix A and determine if the
	// the matrix is singular as limited by the double precision
	// floating-point data representation.

    pos = neg = Scalar::zero;
    temp =  (source[0][0] * source[1][1] * source[2][2]);
    ACCUMULATE
    temp =  (source[0][1] * source[1][2] * source[2][0]);
    ACCUMULATE
    temp =  (source[0][2] * source[1][0] * source[2][1]);
    ACCUMULATE
    temp = -(source[0][2] * source[1][1] * source[2][0]);
    ACCUMULATE
    temp = -(source[0][1] * source[1][0] * source[2][2]);
    ACCUMULATE
    temp = -(source[0][0] * source[1][2] * source[2][1]);
    ACCUMULATE
    Scalar det = pos + neg;

	AssertMsg((det-Scalar::one).Abs() < EPSILON,"det = " << det <<  ",det-one = " << (det-Scalar::one).Abs() <<", EPSILON = " << EPSILON << ", source matrix = " << source);

#endif

    (*this)[0][0] =  Mac2(source[1][1],source[2][2], -source[1][2],source[2][1]);
    (*this)[1][0] = -Mac2(source[1][0],source[2][2], -source[1][2],source[2][0]);
    (*this)[2][0] =  Mac2(source[1][0],source[2][1], -source[1][1],source[2][0]);
    (*this)[0][1] = -Mac2(source[0][1],source[2][2], -source[0][2],source[2][1]);
    (*this)[1][1] =  Mac2(source[0][0],source[2][2], -source[0][2],source[2][0]);
    (*this)[2][1] = -Mac2(source[0][0],source[2][1], -source[0][1],source[2][0]);
    (*this)[0][2] =  Mac2(source[0][1],source[1][2], -source[0][2],source[1][1]);
    (*this)[1][2] = -Mac2(source[0][0],source[1][2], -source[0][2],source[1][0]);
    (*this)[2][2] =  Mac2(source[0][0],source[1][1], -source[0][1],source[1][0]);

	// Calculate -C * inverse(source)
    (*this)[3][0] = -Mac3(source[3][0],(*this)[0][0], source[3][1],(*this)[1][0], source[3][2],(*this)[2][0]);
    (*this)[3][1] = -Mac3(source[3][0],(*this)[0][1], source[3][1],(*this)[1][1], source[3][2],(*this)[2][1]);
    (*this)[3][2] = -Mac3(source[3][0],(*this)[0][2], source[3][1],(*this)[1][2], source[3][2],(*this)[2][2]);
}

// ------------------------------------------------------------------------

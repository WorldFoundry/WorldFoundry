
#define _EULER_CC

// ------------------------------------------------------------------------

#include "global.hp"
#include "euler.hp"
//#include "matrix34.hp"
//#include "math.h"

// ------------------------------------------------------------------------

Euler::Euler( Order order )
	:
	_order( order )
{
	SetA( 0 );
	SetB( 0 );
	SetC( 0 );
}

// ------------------------------------------------------------------------

Euler::Euler( Order order, double a, double b, double c )
	:
	_order( order )
{
	SetA( a );
	SetB( b );
	SetC( c );
}

// ------------------------------------------------------------------------

struct AxisOrder
{
	int axis0;
	int axis1;
	int axis2;
};

AxisOrder OrderAxes[] =
{
	{0,1,2},
	{1,2,0},
	{2,0,1},
	{0},
	{0,2,1},
	{1,0,2},
	{2,1,0},
	{0},
	{0,1,2},
	{1,2,0},
	{2,0,1},
	{0},
	{0,2,1},
	{1,0,2},
	{2,1,0},
	{0},
	{0,1,2},
	{1,2,0},
	{2,0,1},
	{0},
	{0,2,1},
	{1,0,2},
	{2,1,0},
	{0},
	{0,1,2},
	{1,2,0},
	{2,0,1},
	{0},
	{0,2,1},
	{1,0,2},
	{2,1,0},
	{0}
};

// ------------------------------------------------------------------------

Euler::Euler( Euler::Order order, const Matrix3& m )
	:
	_order( order )
{
	int axis0 = OrderAxes[order].axis0;
	int axis1 = OrderAxes[order].axis1;
	int axis2 = OrderAxes[order].axis2;

	if( order & Euler::REPEAT )
	{
		double sy = sqrt( m[axis1][axis0] * m[axis1][axis0] + m[axis2][axis0] * m[axis2][axis0] );
		if( sy > 0.0001 )
		{
			SetA( atan2( m[axis1][axis0], m[axis2][axis0] ) );
			SetB( atan2( sy, (double)m[axis0][axis0] ) );
			SetC( atan2( m[axis0][axis1], -m[axis0][axis2] ) );
		}
		else
		{
			SetA( atan2( -m[axis2][axis1], m[axis1][axis1] ) );
			SetB( atan2( sy, (double)m[axis0][axis0] ) );
			SetC( 0 );
		}
	}
	else
	{
		double cy = sqrt( m[axis0][axis0] * m[axis0][axis0] + m[axis0][axis1] * m[axis0][axis1] );
		if( cy > 0.0001 )
		{
			SetA( atan2( m[axis1][axis2], m[axis2][axis2] ) );
			SetB( atan2( (double)-m[axis0][axis2], cy ) );
			SetC( atan2( m[axis0][axis1], m[axis0][axis0] ) );
		}
		else
		{
			SetA( atan2( -m[axis2][axis1], m[axis1][axis1] ) );
			SetB( atan2( (double)-m[axis0][axis2], cy ) );
			SetC( 0 );
		}
	}

	if( order & Euler::EULER_PARITY )
	{
		SetA( -GetA() );
		SetB( -GetB() );
		SetC( -GetC() );
	}

	if( order & Euler::FRAME )
	{
		double t;

		t  = GetA();
		SetA( GetC() );
		SetC( t );
	}
}

// ------------------------------------------------------------------------

double&
Euler::operator [] ( int idxAxis )
{
	assert( idxAxis >= 0 );
	assert( idxAxis < 3 );
	return _axes[idxAxis];
}

// ------------------------------------------------------------------------

const double&
Euler::operator [] ( int idxAxis ) const
{
	assert( idxAxis >= 0 );
	assert( idxAxis < 3 );
	return _axes[idxAxis];
}

// ------------------------------------------------------------------------

void
Euler::AsMatrix3( Matrix3& m ) const
{
	double ti, tj, th;
	if( GetOrder() & Euler::FRAME )
	{
		ti = GetC(); tj = GetB(); th = GetA();
	}
	else
	{
		ti = GetA(); tj = GetB(); th = GetC();
	}

	if( GetOrder() & Euler::EULER_PARITY )
	{
		ti = -ti; tj = -tj; th = -th;
	}

	double ci = cos( ti );
	double cj = cos( tj );
	double ch = cos( th );
	double si = sin( ti );
	double sj = sin( tj );
	double sh = sin( th );
	double cc = ci * ch;
	double cs = ci * sh;
	double sc = si * ch;
	double ss = si * sh;

	int axis0 = OrderAxes[GetOrder()].axis0;
	int axis1 = OrderAxes[GetOrder()].axis1;
	int axis2 = OrderAxes[GetOrder()].axis2;

	if( GetOrder() & Euler::REPEAT )
	{
 		m[axis0][axis0] = cj;		m[axis1][axis0] = sj * si;			m[axis2][axis0] = sj * ci;
 		m[axis0][axis1] = sj * sh;	m[axis1][axis1] = -cj * ss + cc;	m[axis2][axis1] = -cj * cs - sc;
 		m[axis0][axis2] = -sj * ch;	m[axis1][axis2] = cj * sc + cs;		m[axis2][axis2] = cj * cc - ss;
	}
	else
	{
		m[axis0][axis0] = cj * ch;	m[axis1][axis0] = sj * sc - cs;		m[axis2][axis0] = sj * cc + ss;
		m[axis0][axis1] = cj * sh;	m[axis1][axis1] = sj * ss + cc;		m[axis2][axis1] = sj * cs - sc;
		m[axis0][axis2] = -sj;		m[axis1][axis2] = cj * si;			m[axis2][axis2] = cj * ci;
	}

	m[3] = Point3( 0, 0, 0 );
}

// ------------------------------------------------------------------------

int
Euler::operator == ( const Euler& x ) const
{
	return
		( GetOrder() == x.GetOrder() )
		&& ( GetA() == x.GetA() )
		&& ( GetB() == x.GetB() )
		&& ( GetC() == x.GetC() );
}

// ------------------------------------------------------------------------

int
Euler::operator != ( const Euler& x ) const
{
	return !( (*this) == x );
}

// ------------------------------------------------------------------------

int
Euler::operator < ( const Euler& x ) const
{
	if( int( GetOrder() ) < int( x.GetOrder() ) ) return true;
	else if( int( GetOrder() ) > int( x.GetOrder() ) ) return false;
	else
	{
		if( GetA() < x.GetA() ) return true;
		else if( GetA() > x.GetA() ) return false;
		else
		{
			if( GetB() < x.GetB() ) return true;
			else if( GetB() > x.GetB() ) return false;
			else
			{
				if( GetC() < x.GetC() ) return true;
				else return false;
			}
		}
	}
}

// ------------------------------------------------------------------------

int
Euler::operator <= ( const Euler& x ) const
{
	return ( (*this) < x ) || ( (*this) == x );
}

// ------------------------------------------------------------------------

int
Euler::operator > ( const Euler& x ) const
{
	return x < (*this);
}

// ------------------------------------------------------------------------

int
Euler::operator >= ( const Euler& x ) const
{
	return ( (*this) > x ) || ( (*this) == x );
}

// ------------------------------------------------------------------------

ostream&
operator << ( ostream& os, const Euler& x )
{
	return os << x.GetA() << " " << x.GetB() << " " << x.GetC();
}

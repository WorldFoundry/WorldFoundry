// ccycle.cc

#include <gfxfmt/bitmap.hp>

ColourCycle::ColourCycle()
{
	_start = _end = ~0;
	_speed = 0.0;
}


ColourCycle::ColourCycle( int start, int end, float speed )
{
	_start = start;
	_end = end;
	_speed = speed;
}


ColourCycle::~ColourCycle()
{
}


bool
ColourCycle::operator==( const ColourCycle& cc ) const
{
	return ( _start == cc._start ) && ( _end == cc._end ) && ( _speed == cc._speed );
}


bool
ColourCycle::operator!=( const ColourCycle& cc ) const
{
	return !( *this == cc );
}

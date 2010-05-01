// clock.cc

#include <timer/clock.hp>

Clock::Clock()
{
	reset();
}


Clock::~Clock()
{
}


void
Clock::Tick( Scalar time )
{
	_lastDeltaClock = _deltaClock;

	if (_nWallClock == Scalar::zero )
		_deltaClock = SCALAR_CONSTANT(1.0/10.0);
	else
		_deltaClock = time - _nWallClock;

	_nWallClock = time;
}

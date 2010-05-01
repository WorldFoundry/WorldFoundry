// health.cc

#include "health.hp"

//==============================================================================

Health::Health( Scalar nHitPoints )
{
	_nHitPoints = nHitPoints;
	_pHealth = NULL;
}

//==============================================================================

Health::~Health()
{
}

//==============================================================================

Scalar
Health::operator()() const
{
	return _nHitPoints;
}

//==============================================================================

void
Health::deltaHealth( Scalar nHitPoints )
{
	_nHitPoints += nHitPoints;
}

//==============================================================================

const Health*
Health::next() const
{
	return _pHealth;
}

//==============================================================================

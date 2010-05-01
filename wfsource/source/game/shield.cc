//============================================================================
// Shield.cc:
// Copyright ( c ) 1996,1997,1999,2000,2002,2003 World Foundry Group  
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

//=============================================================================
//	Abstract:
//	History:
//			Created	From object.ccs using Prep
//
//	Class Hierarchy:
//			none
//
//	Dependancies:
//
//	Restrictions:
//
//	Example:
//
//============================================================================

// Shield options --
//		max damage absorbed
//		absorb all damage
// Questions --
//		Q: how do I pass in who this shield belongs to (to follow)
//		A: create a new Actor constructor which attaches the VisualActor to a parent
//		Q: read oad fields from the script?

#include <oas/shield.ht>
#include <movement/movefoll.hp>
#include "shield.hp"

//============================================================================

extern FollowHandler theFollowHandler;

Shield::Shield( const SObjectStartupData* startupData ) : Actor( startupData )
{
	_owner = startupData->idxCreator;
	assert( _owner > 0 );

//	GetMailboxes().WriteMailbox(EMAILBOX_SOUND, 38);

	GetMovementManager().SetMovementHandler( &theFollowHandler, *this );
	((FollowHandler*)&GetMovementManager().GetMovementHandler(*this))->SetTrackedObject( this, _owner );

	Actor* pOwner = theLevel->getActor( _owner ) ;
	AssertMsg( pOwner->kind() == Actor::Player_KIND, "Currently only Player can use shield: " << *pOwner << " tried to and can't" << std::endl );
#pragma message ("KTS: is there a better way to access a base class protected member through a pointer?")
	((Shield*)pOwner)->_nonStatPlat->_shield = this;
	_shieldLevel = 0;
	_invulnerableUntil = Scalar::zero;
	++(*this);
}

//============================================================================

void
Shield::operator++()
{
//	GetMailboxes().WriteMailbox(EMAILBOX_SOUND, 44);
	++_shieldLevel;
	_displayUntil = theLevel->LevelClock().Current() + getOad()->GetShieldPurchaseDisplay();
	_toggleWhen = theLevel->LevelClock().Current() + getOad()->GetBlinkFrequency();
	_bDisplayed = true;
}

//============================================================================

void
Shield::operator--()
{
	assert( _shieldLevel > 0 );
//	GetMailboxes().WriteMailbox(EMAILBOX_SOUND, 39);
	--_shieldLevel;
	_displayUntil = theLevel->LevelClock().Current() + getOad()->GetShieldPurchaseDisplay();
	_toggleWhen = theLevel->LevelClock().Current() + getOad()->GetBlinkFrequency();
	_bDisplayed = true;
#if 0
	if ( _shieldLevel == 0 )
	{
		theLevel->SetPendingRemove( this );
		Actor* pActor = theLevel->getActor( _owner );
		assert( ValidPtr( pActor ) );
		pActor->_nonStatPlat->_shield = NULL;
	}
#endif
	_invulnerableUntil = theLevel->LevelClock().Current() + Scalar( getOad()->GetInvulnerabilityDisplay() );
}

//============================================================================

bool
Shield::isVisible() const
{	// calls non-const version of isVisible()
	return ((Shield*)this)->isVisible();
}

//============================================================================

bool
Shield::isVisible()
{
	Scalar wallclock = theLevel->LevelClock().Current();

	if ( wallclock <= _displayUntil )
	{
		if ( wallclock >= _toggleWhen )
		{
			_bDisplayed = !_bDisplayed;
			_toggleWhen += getOad()->GetBlinkFrequency();
		}
	}
	else
	{
		_bDisplayed = false;
		if ( _shieldLevel == 0 )
			theLevel->SetPendingRemove( this );
	}

	return _bDisplayed && Actor::isVisible();
}

//============================================================================

bool
Shield::isInvulnerable() const
{
	return theLevel->LevelClock().Current() <= _invulnerableUntil;
}

//============================================================================

Actor::EActorKind
Shield::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Shield_KIND);
	return Actor::Shield_KIND;
}

//============================================================================

void
Shield::update()
{
	Actor* pOwner = theLevel->getActor( _owner );
	assert( ValidPtr( pOwner ) );

	if ( !pOwner->hasRunUpdate() )
		pOwner->update();

	Actor::update();

	setCurrentPos( pOwner->currentPos() );
}

//============================================================================

Actor*
OadShield( const SObjectStartupData* startupData )
{
	Actor* theActor = new (*startupData->memory) Shield( startupData );
	assert( theActor );
	return theActor;
}

//============================================================================

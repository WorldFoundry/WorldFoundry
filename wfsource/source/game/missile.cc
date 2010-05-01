//=============================================================================
// missile.cc:
// Copyright ( c ) 1994,1995,1996,1997,1999,2000,2002,2003 World Foundry Group  
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
// Description: The Missile class implements missiles which may be thrown
// 				by actors in the world
// Original Author: Ann Mei Chang (grenade.cc)
// 			Updated KTS 11-11-95 08:06pm to use new template system
//                  LDH 01-19-96 cloned for Missile.cc
//=============================================================================

#define _MISSILE_CC

#include "level.hp"
#include "missile.hp"
#include "oas/missile.ht"		// get oad structure information
#include "explode.hp"
#include <math/scalar.hp>
#include <movement/movement.hp>
#include "gamestrm.hp"
#include <movement/inputmap.hp>

//==============================================================================

Missile::Missile(const SObjectStartupData* startupData) : Actor(startupData)
{
	_armed = false;
	_timeToArm = startupData->currentTime.Current() + Scalar(getOad()->GetArmingDelay());
	_timeToExplode = startupData->currentTime.Current() + Scalar(getOad()->GetExplosionDelay());
//	_objectToCreate = GetCommonBlockPtr()->Poof;
//	AssertMsg( _objectToCreate, "Missile object " << *this << " doesn't have an objectToThrow reference" );
	_owner = startupData->idxCreator;
}

//==============================================================================

Missile::~Missile()
{
}

//==============================================================================

Actor::EActorKind
Missile::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Missile_KIND);
	return Actor::Missile_KIND;
}

//==============================================================================
// note: any member function calling explode must return immediately
// afterwards, since the object has been deleted missile explodes - remove
// from world and create an explosion (or whatever object the designer
// has chosen)
void
Missile::explode()
{
	theLevel->SetPendingRemove( this );

#pragma message( __FILE__ ": this is where a message would be sent to script" )
	_nonStatPlat->_hitPoints = Scalar::zero;
}

//==============================================================================

void
Missile::predictPosition(const Clock& currentTime)
{
	Actor::predictPosition(currentTime);
}

//==============================================================================

void
Missile::update()
{
	bool explodeNow = false;

	_armed = theLevel->LevelClock().Current() >= _timeToArm;

   char msgData[msgDataSize];

	while ( _nonStatPlat->_msgPort.GetMsgByType( MsgPort::SPECIAL_COLLISION, &msgData, msgDataSize ) )
		{
		DBSTREAM1( ctool << "Missile got special collision" << std::endl; )
		if ( _armed && getOad()->ExplodeOnImpact )				// if ready to explode, explode
			explodeNow = true;
		else
			_nonStatPlat->_msgPort.PutMsg( MsgPort::COLLISION, msgData,msgDataSize );		// otherwise, convert this into a regular collision message
		}

	if ( _armed )
	{
#pragma message ("KTS: stop using mainChacter")
		Actor* main = dynamic_cast<Actor*>(theLevel->mainCharacter());
      assert(ValidPtr(main));
		if ( main->GetInputDevice()->justReleased( kBtnGrenade ) )
			explodeNow = true;

		if ( theLevel->LevelClock().Current() >= _timeToExplode )
			explodeNow = true;
	}

	if ( explodeNow )
		explode();

	Actor::update();
}

//==============================================================================

Actor*
OadMissile( const SObjectStartupData* startupData )
{
	return new (*startupData->memory) Missile(startupData);
}

//============================================================================

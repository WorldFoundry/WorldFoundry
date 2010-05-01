//============================================================================
// Enemy.cc:
// Copyright ( c ) 1995,1996,1997,1998,1999,2000,2002,2003 World Foundry Group  
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

#include "actor.hp"
#include "enemy.hp"
#include "actor.hp"
#include "oas/enemy.ht"
#include "camera.hp"
#include "gamestrm.hp"

//============================================================================

Enemy::Enemy( const SObjectStartupData* startupData ) : Actor( startupData )
{
}

//============================================================================

Enemy::~Enemy()
{
}

//============================================================================

Actor::EActorKind
Enemy::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Enemy_KIND);
	return Actor::Enemy_KIND;
}

//============================================================================

void
Enemy::update()
{
   char msgData[msgDataSize];

	while ( _nonStatPlat->_msgPort.GetMsgByType( MsgPort::SPECIAL_COLLISION, &msgData, msgDataSize ) )
	{
		Actor* colActor = (Actor*)msgData;
		assert(ValidPtr(colActor));
//		colActor->sendMsg(MsgPort::DELTA_HEALTH, getOad()->NumberOfGold);
		DBSTREAM1( cerror << "guard doing damage" << std::endl; )
		colActor->sendMsg(MsgPort::DELTA_HEALTH, -10);
		// else throw away collision message
	}

	Actor::update();
}

//============================================================================

Actor*
OadEnemy( const SObjectStartupData* startupData )
{
	Actor* theActor = new (*startupData->memory) Enemy( startupData );
	assert( theActor );
	return theActor;
}

//============================================================================

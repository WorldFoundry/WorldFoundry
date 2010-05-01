//============================================================================
// ActBox.cc:
// Copyright ( c ) 1995,1996,1997,1999,2000,2002,2003 World Foundry Group  
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

#define _ACTBOX_CC

#include <oas/levelcon.h>
#include <oas/actbox.ht>
#include <oas/activate.ht>
#include "actbox.hp"
#include "actor.hp"
#include <physics/activate.hp>
#include "gamestrm.hp"
#include "level.hp"

//============================================================================

inline const _Activation*
ActBox::GetActivateBlockPtr() const
{
   assert(ValidPtr(getOad()));
   assert(ValidPtr(GetBlockPtr(getOad()->activatePageOffset)));
   return (_Activation*)GetBlockPtr(getOad()->activatePageOffset);
}
    
//==============================================================================
    
    
ActBox::ActBox(const SObjectStartupData* startupData) : 
  Actor(startupData),
  activation(*GetActivateBlockPtr(), GetCommonBlock())
{

#if 0
	int32* pObjList = _objList;
	while ( pObjList && *pObjList != -1 )
	{
		DBSTREAM1( cnull << "actor index " << *pObjList << std::endl; )
		++pObjList;
	}
#endif
}

//============================================================================

void
ActBox::activate( const Actor* pActor )
{
    assert( ValidPtr( pActor ) );
    DBSTREAM1( cactor << "actbox update: writing " << getOad()->MailBoxValue << " to mailbox " << getOad()->MailBox << std::endl; )
    GetMailboxes().WriteMailbox( getOad()->MailBox, Scalar::FromFixed32( getOad()->MailBoxValue ) );
    GetMailboxes().WriteMailbox( getOad()->ActivatedActorMailbox, Scalar( pActor->GetActorIndex(), 0 ) );
}

//============================================================================

void
ActBox::doFieldEffect( BaseObject& pColObject )
{
    DBSTREAM1( cactor << "actbox::doFieldEffect: ");

	DBSTREAM1( cactor << "Vector X: " << getOad()->VectorX; )
	DBSTREAM1( cactor << ", Vector Y: " << getOad()->VectorY; )
	DBSTREAM1( cactor << ", Vector Z: " << getOad()->VectorZ; )
	int count = 0;

   Scalar temp;

	if ( (temp = getOad()->GetVectorX()).AsBool() )
	{
		pColObject.sendMsg( MsgPort::MOVEMENT_FORCE_X, &temp,sizeof(Scalar) );
		count++;
	}

	if ( (temp = getOad()->GetVectorY()).AsBool() )
	{
		pColObject.sendMsg( MsgPort::MOVEMENT_FORCE_Y, &temp,sizeof(Scalar) );
		count++;
	}

	if ( (temp = getOad()->GetVectorZ()).AsBool() )
	{
		pColObject.sendMsg( MsgPort::MOVEMENT_FORCE_Z, &temp,sizeof(Scalar) );
		count++;
	}
}

//============================================================================

void
ActBox::update()
{
	if ( !GetMailboxes().ReadMailbox( getOad()->ActivationMailbox ).AsBool() )
		return;

	if ( getOad()->ClearOnExit )
    {
		GetMailboxes().WriteMailbox( getOad()->MailBox, getOad()->GetMailboxExitValue() );
        GetMailboxes().WriteMailbox( getOad()->ActivatedActorMailbox, Scalar::zero );
    }

	DBSTREAM1( cnull << "actbox::update on " << *this << std::endl; )

	BaseObject* colObject = activation.Activated( GetPhysicalAttributes(),theLevel->GetActiveRooms().GetObjectIter(ROOM_OBJECT_LIST_COLLIDE), *GetActivateBlockPtr(), theLevel->GetObjectList());
	if ( colObject )
	{
		assert( ValidPtr( colObject ) );
      Actor* colActor = dynamic_cast<Actor*>(colObject);
      assert(ValidPtr(colActor));
      activate( colActor );
		doFieldEffect( *colObject );
	}
	Actor::update();
	// FIX -- hack until order execution priority is enabled because actboxes
	// are executed before all other objects (and prepareToRender isn't called at all)
	GetWritablePhysicalAttributes().HasRunUpdate(false);
}

//============================================================================

Actor::EActorKind
ActBox::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::ActBox_KIND);
	return Actor::ActBox_KIND;
}

//============================================================================

Actor*
OadActBox( const SObjectStartupData* startupData )
{
	return new (*startupData->memory) ActBox(startupData);
}

//============================================================================

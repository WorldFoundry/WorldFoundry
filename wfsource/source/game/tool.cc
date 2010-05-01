//============================================================================
// Tool.cc:
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

#include <math/scalar.hp>
#include <oas/tool.ht>
#include "actor.hp"
#include "tool.hp"
#include "toolngun.hp"
#include "toolshld.hp"
#include "gamestrm.hp"
#include "shield.hp"

//============================================================================

Tool::Tool( const SObjectStartupData* startupData ) : Actor( startupData )
{
	const _Tool *toolPtr = getOad();
	AssertMsg( ValidPtr( toolPtr ), *this << ": Missing tool data" << std::endl );
	_toolOAD = *toolPtr;
	_objectToGenerate = getOad()->ObjectToThrow;
	_timeAvailableToFire = theLevel->LevelClock().Current();
	_InitActivationScript( startupData );
	_memory = startupData->memory;
	Validate();
}

//============================================================================

void
Tool::_InitActivationScript( const SObjectStartupData* startupData )
{
	_pScript = NULL;
	if ( getOad()->ActivationScript != -1 )
	{
		DBSTREAM1( cdebug << "GetOad()->ActivationScript = " << getOad()->ActivationScript << std::endl; )
		_pScript = startupData->commonBlock->GetBlockPtr(getOad()->ActivationScript);
		AssertMsg( ValidPtr( _pScript ), "actor = " << *this );
	}
}

//============================================================================

Tool::~Tool()
{
	Validate();
}

//============================================================================

Actor::EActorKind
Tool::kind() const
{
	Validate();
   assert(GetMovementBlockPtr()->MovementClass == Actor::Tool_KIND);
	return Actor::Tool_KIND;
}

//==============================================================================

bool
Tool::CanRender() const
{
	Validate();
	return false;
}

//==============================================================================

bool
Tool::CanUpdate() const
{
	Validate();
	return false;
}

//==============================================================================

const Vector3&
Tool::currentPos() const
{
	Validate();
	if (_owner == 0)
	{
		DBSTREAM1( ctool << "No owner, using default Pos"; )
		return(GetPhysicalAttributes().Position());
	}
	else
	{
		assert(_owner);
		assert(ValidPtr(theLevel->getActor(_owner)));
		return(theLevel->getActor(_owner)->GetPhysicalAttributes().PredictedPosition());
	}
}

//==============================================================================

const Vector3&
Tool::GetSpeed() const
{
	Validate();
	if (_owner == 0)
	{
		return(GetPhysicalAttributes().LinVelocity());
	}
	else
	{
		assert(ValidPtr(theLevel->getActor(_owner)));
		return(theLevel->getActor(_owner)->GetPhysicalAttributes().LinVelocity());
	}
}

//==============================================================================

Vector3
Tool::currentDir() const
{
	Validate();
	if (_owner == 0)
	{
		DBSTREAM1( ctool << "No owner, using default cDir"; )
		return(Actor::currentDir());
	}
	else
	{
		assert(ValidPtr(theLevel->getActor(_owner)));
		return(theLevel->getActor(_owner)->currentDir());
	}
}

//==============================================================================

const PhysicalAttributes&
Tool::GetPhysicalAttributes() const
{
	Validate();
	if (_owner == 0)
	{
		return(Actor::GetPhysicalAttributes());
	}
	else
	{
		assert(ValidPtr(theLevel->getActor(_owner)));
		return(theLevel->getActor(_owner)->GetPhysicalAttributes());
	}
}

//============================================================================

PhysicalAttributes&
Tool::GetWritablePhysicalAttributes()					// get non-const so we can change it
{
	Validate();
	if (_owner == 0)
	{
		return(Actor::GetWritablePhysicalAttributes());
	}
	else
	{
		assert(ValidPtr(theLevel->getActor(_owner)));
		return(theLevel->getActor(_owner)->GetWritablePhysicalAttributes());
	}
}

//============================================================================

bool
Tool::canActivate()
{
	Validate();
	assert( _owner );
	assert(ValidPtr(theLevel->getActor(_owner)));
	return
//		( theLevel->getActor(_owner)->numGold() >= getOad()->ActivationCost ) &&
		( theLevel->LevelClock().Current() >= _timeAvailableToFire );
}

//============================================================================

void
Tool::activate()
{
	Validate();
	DBSTREAM1( ctool << "activated tool type "; )
	DBSTREAM1( ctool << getOad()->Type << std::endl; )

	if ( getOad()->Type == TOOL_TYPE_PROJECTILE )
	{
		assert( _owner );
		assert(ValidPtr(theLevel->getActor(_owner)));
		assert( 0 <= _toolOAD.Type && _toolOAD.Type < TOOL_COUNT );

#pragma message( "Tools now free because gold is gone" )
#if 0
		assert( theLevel->getActor(_owner)->numGold() >= getOad()->ActivationCost );
		theLevel->getActor(_owner)->removeGold( getOad()->ActivationCost );
#endif

		_timeAvailableToFire = theLevel->LevelClock().Current() + getOad()->GetRecharge();

		// it's time to spew an object
		const Vector3 normxy = currentDir();
		DBSTREAM1( ctool << normxy.X() << "," << normxy.Y() << std::endl; )
		const Vector3& _speed = GetSpeed() * (Scalar::FromFixed32(getOad()->MovingThrowPercentage) / SCALAR_CONSTANT(100));
		Vector3 dir(
			(_speed.X() + (normxy.X() * Scalar(_toolOAD.GetHorizSpeed()))),
			(_speed.Y() + (normxy.Y() * Scalar(_toolOAD.GetHorizSpeed()))),
			(_speed.Z() + Scalar(_toolOAD.GetVertSpeed())) );

		DBSTREAM1( if (dir.X() == Scalar::zero && dir.Y() == Scalar::zero )
			cerror << "Tool::Activate: no forward speed on object!" << std::endl; )

		SObjectStartupData* startupData = (SObjectStartupData*)theLevel->FindTemplateObjectData(_objectToGenerate);
		assert(startupData);

		Actor* toolOwner = theLevel->getActor(_owner);
		Vector3 gunLocation;
#if DO_ASSERTIONS
		bool validHandle =
#endif
			toolOwner->GlobalHandleLocation( HandleID("lnch"), gunLocation );
		AssertMsg( validHandle, "This Animation doesn't have a 'lnch' handle! in "  << *toolOwner);
		const PhysicalAttributes& ownerPA = toolOwner->GetPhysicalAttributes();

		// Create a colBox for the template object (where it wants to get constructed)
		_CollisionRectOnDisk& colRect = startupData->objectData->coarse;

#if 1
		Vector3 vect1( Scalar::FromFixed32(colRect.minX), Scalar::FromFixed32(colRect.minY), Scalar::FromFixed32(colRect.minZ) );
		Vector3 vect2( Scalar::FromFixed32(colRect.maxX), Scalar::FromFixed32(colRect.maxY), Scalar::FromFixed32(colRect.maxZ) );
		ColBox templateColBox( vect1,
						   	   vect2 );
#else
// kts 8/4/99 gcc2.95 can't handle this, figure out why
		ColBox templateColBox( Vector3( Scalar(colRect.minX), Scalar(colRect.minY), Scalar(colRect.minZ) ),
						   	   Vector3( Scalar(colRect.maxX), Scalar(colRect.maxY), Scalar(colRect.maxZ) ) );
#endif
		templateColBox.Expand( dir );

		// Push it far enough along it's velocity vector to make it clear the owner's colBox
		Scalar offset;
		if ( currentDir().X().Abs() < currentDir().Y().Abs() )
		{	// off of y face
			if ( currentDir().Y() < Scalar::zero )		// MinY
				offset = templateColBox.Max(gunLocation).Y() - ownerPA.Min().Y();
			else	// MaxY
				offset = ownerPA.Max().Y() - templateColBox.Min( gunLocation).Y();

			offset += SCALAR_CONSTANT(0.01);
			if ( offset > Scalar::zero )
				gunLocation += dir * (offset / dir.Y()).Abs();
		}
		else
		{	// off of x face
			if ( currentDir().X() < Scalar::zero )		// MinX
				offset = templateColBox.Max(gunLocation).X() - ownerPA.Min().X();
			else	// MaxX
				offset = ownerPA.Max().X() - templateColBox.Min(gunLocation).X();

			offset += SCALAR_CONSTANT(0.01);
			if ( offset > Scalar::zero )
				gunLocation += dir * (offset / dir.X()).Abs();
		}


		Actor* createdObject = theLevel->ConstructTemplateObject(_objectToGenerate,_owner, gunLocation, dir);

		if (createdObject)
		{
			DBSTREAM1( ctool << "tool: Creating object type " << createdObject->kind() << std::endl; )
			dir += toolOwner->GetPhysicalAttributes().LinVelocity();
			createdObject->setSpeed( dir );
			createdObject->GetWritablePhysicalAttributes().SetRotation( GetPhysicalAttributes().Rotation() );

			theLevel->AddObject( createdObject, gunLocation );
			createdObject->GetMovementManager().InitMovementHandler(*this);
		}
	}

	// Run this tool's activation script (if it has one)
	if (_pScript)
      theLevel->EvalScript(_pScript,GetActorIndex());
}

//============================================================================

void
Tool::trigger( const QInputDigital& input )
{
	Validate();
	static joystickButtonsF buttonMap[TOOL_BUTTON_COUNT] =
	{
		EJ_BUTTONF_A,	// button A
		EJ_BUTTONF_B,	// button B
		EJ_BUTTONF_C,	// button C
		EJ_BUTTONF_D,	// button D
		EJ_BUTTONF_E,	// button E
		EJ_BUTTONF_F	// button F
	};
	joystickButtonsF buttons = getOad()->Autofire ? input.arePressed() : input.justPressed();

//	std::cout << hex << "Checking button press of " << buttons << " version tool button " << buttonMap[_toolOAD.ActivationButton] << dec << std::endl;
	if ( buttonMap[_toolOAD.ActivationButton] == (buttons & buttonMap[_toolOAD.ActivationButton]) )
	{
		DBSTREAM1( ctool << getOad()->Type << std::endl; )
		if ( canActivate() )
		{
			DBSTREAM1( ctool << "triggered tool type "; )
			activate();
		}
	}
}

//============================================================================

Actor*
OadTool( const SObjectStartupData* startupData )
{
	assert( ValidPtr( startupData ) );
	const _Tool* toolPtr = (_Tool*)ObjectGetOAD( startupData );
	AssertMsg( ValidPtr( toolPtr ), "Missing tool data" );

	DBSTREAM1( ctool << "creating tool type "; )
	DBSTREAM1( ctool << toolPtr->Type << std::endl; )

	switch ( toolPtr->Type )
	{
		case Tool::TOOL_TYPE_BEAM:
			return new (*startupData->memory) ToolNeedleGun(startupData);

		case Tool::TOOL_TYPE_PROJECTILE:
			return new (*startupData->memory) Tool(startupData);

		case Tool::TOOL_TYPE_SHIELD:
			return new (*startupData->memory) ToolShield( startupData );

		default:
			DBSTREAM1( ctool << "Tool type " << toolPtr->Type << " found." << std::endl; )
			AssertMsg(0, "Bad tool type\n");
			return NULL;
	}
}

//============================================================================

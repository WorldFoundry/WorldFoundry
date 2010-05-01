//==============================================================================
// Player.cc:
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

//==============================================================================
// Description: The Player class is our main character.
// Original Author: Kevin T. Seghetti
//==============================================================================

#define _PLAYER_CC

#include <anim/path.hp>
#include <math/angle.hp>
#include <oas/player.ht>
#include <oas/movement.h>
#include <movement/inputmap.hp>
#include <pigsys/minmax.hp>
#include "gamestrm.hp"
#include "camera.hp"
#include "player.hp"

//==============================================================================

Player::Player( const SObjectStartupData* startupData ) : Actor(startupData)
{
//	_startingPos = Vector3(
//		Scalar( startupData->objectData->x ),
//		Scalar( startupData->objectData->y ),
//		Scalar( startupData->objectData->z ) );
//	_startingHp = GetCommonBlockPtr()->hp;

//	AssertMsg( GetCommonBlockPtr()->ScriptControlsInput, "Script Controls Input must be turned on for player" );
//	AssertMsg( GetCommonBlockPtr()->Script != -1, "Player must have a script which at least copies input from joystick" );
}

//==============================================================================

Player::~Player()
{
}

//==============================================================================

Actor::EActorKind
Player::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Player_KIND);
	return Actor::Player_KIND;
}

//==============================================================================

void
Player::die()
{
#if 0
	Continue* continuationMarker = theLevel->getContinue();
	if ( continuationMarker )
	{
		continuationMarker->use();
		_hitPoints = _startingHp;
	}
	else
#endif
	{
		GetMailboxes().WriteMailbox( EMAILBOX_END_OF_LEVEL, Scalar::one );
#pragma message( __FILE__ ": use FindOADData to get starting position and other information" )
//		GetWritablePhysicalAttributes().SetPosition( _startingPos );
//		GetWritablePhysicalAttributes().SetPredictedPosition( _startingPos );
//		_hitPoints = _startingHp;
	}
}

//==============================================================================

void
Player::update()
{
	int16 msgType;
   char msgData[msgDataSize];

	joystickButtonsF buttons = _nonStatPlat->_input->arePressed();

#ifdef DIRECTORY_VIEW_TOOL
	if ( (getOad()->Directory != -1) && bFirstTime )
	{
		bFirstTime = false;

		struct find_t  fileinfo;
		unsigned rc;        // return code

		Vector3 startPos = currentPos();

		// Display name and size of "*.c" files
		Scalar x = Scalar::zero;
		Scalar y = Scalar::zero;
		rc = _dos_findfirst( "*.*", _A_NORMAL | _A_SUBDIR, &fileinfo );
		while( rc == 0 )
		{
			Actor* createdObject;

			strupr( fileinfo.name );

			if ( fileinfo.attrib & _A_SUBDIR )
			{
				SObjectStartupData* dirStartupData = (SObjectStartupData*)
					theLevel->FindTemplateObjectData( getOad()->Directory );
				assert( ValidPtr( dirStartupData ) );

				createdObject = new Dir( dirStartupData, fileinfo );
			}
			else
			{
				SObjectStartupData* fileStartupData = (SObjectStartupData*)
					theLevel->FindTemplateObjectData( getOad()->File );
				assert( ValidPtr( fileStartupData ) );

				createdObject = new File( fileStartupData, fileinfo );
			}
			assert( ValidPtr( createdObject ) );
			Vector3 pos = startPos;
			pos.SetX( startPos.X() + x );
			pos.SetY( startPos.Y() + y );

			level->AddObject( createdObject, pos );

		    rc = _dos_findnext( &fileinfo );

			x += SCALAR_CONSTANT( 6 );
			if ( x >= SCALAR_CONSTANT( 60 ) )
			{
				x = Scalar::zero;
				y += SCALAR_CONSTANT( 6 );
			}
		}
	}
#endif

#if 0
	// Timeporter test code
	if ( _nonStatPlat->_input->justPressed( kBtnStepRight ) )
	{
		assert( getOad()->Timeporter );
		const SObjectStartupData* startupData = theLevel->FindTemplateObjectData( getOad()->Timeporter );
		assert( ValidPtr( startupData ) );
		Actor* createdObject = ConstructTemplateObject( startupData->objectData->type, startupData );
		assert( ValidPtr( createdObject ) );
		//createdObject->setSpeed(_vect);
		theLevel->AddObject( createdObject, currentPos() );
	}

	// TEMP
	if ( _nonStatPlat->_input->justPressed( kBtnUnusedF ) && theLevel->getTimeport() )
		theLevel->getTimeport()->use();
#endif


#if defined(DESIGNER_CHEATS)
	// kts free camera motion code 10-20-95 03:46pm
#if 1
#define FREESPEED SCALAR_CONSTANT(0.5)
#else
// kts 8/4/99 doesn't work with gcc2.95, figure out why
	const Scalar FREESPEED(SCALAR_CONSTANT(0.5));
#endif
	static int freeMode=0;

	if(freeMode)
	{
		GetWritablePhysicalAttributes().HasRunUpdate(true);

		while ( _nonStatPlat->_msgPort.GetMsg( msgType, &msgData, msgDataSize ) )
			; // clean out messages

		// copy input from physical to logical
		GetMailboxes().WriteMailbox(EMAILBOX_INPUT,GetMailboxes().ReadMailbox(EMAILBOX_HARDWARE_JOYSTICK1));

		_physicalAttributes.SetLinVelocity( Vector3::zero );

		if ( _nonStatPlat->_input->justPressed( kBtnAngelMode ) )
		{
			freeMode = false;
			DBSTREAM1( std::cout << "Angel Mode OFF" << std::endl; )
			((_Movement* )GetMovementBlockPtr())->Mobility = MOBILITY_PHYSICS;  // kts nasty
		}

		if ((buttons & EJ_BUTTONF_UP) && !(buttons & kBtnJump))
			_physicalAttributes.SetLinVelocity( Vector3( Scalar::zero-FREESPEED, Scalar::zero, Scalar::zero ) );

		else if ((buttons & EJ_BUTTONF_DOWN) && !(buttons & kBtnJump))
			_physicalAttributes.SetLinVelocity( Vector3( FREESPEED, Scalar::zero, Scalar::zero ) );

		if ((buttons & EJ_BUTTONF_LEFT) && !(buttons & kBtnJump))
			_physicalAttributes.SetLinVelocity(Vector3(Scalar::zero, Scalar::zero-FREESPEED, Scalar::zero ));

		else if ((buttons & EJ_BUTTONF_RIGHT) && !(buttons & kBtnJump))
			_physicalAttributes.SetLinVelocity(Vector3(Scalar::zero, FREESPEED, Scalar::zero ));

		if ((buttons & EJ_BUTTONF_UP) && (buttons & kBtnJump))
			_physicalAttributes.SetLinVelocity(Vector3(Scalar::zero, Scalar::zero, Scalar::zero-FREESPEED));

		else if ((buttons & EJ_BUTTONF_DOWN) && (buttons & kBtnJump))
			_physicalAttributes.SetLinVelocity(Vector3(Scalar::zero, Scalar::zero, FREESPEED));

		_physicalAttributes.SetPredictedPosition(_physicalAttributes.Position() + _physicalAttributes.LinVelocity());		// Apply velocity to pos

		return;
	}
	else
	{
		if ( _nonStatPlat->_input->justPressed( kBtnAngelMode ) )
		{
			DBSTREAM1( std::cout << "Angel Mode ON" << std::endl; )
			((_Movement* )GetMovementBlockPtr())->Mobility = MOBILITY_ANCHORED;  // kts nasty
			freeMode = true;
		}
	}
#endif

	assert( ValidPtr( _nonStatPlat->_animManager ) );		// FIX - only need to to this once

	Actor::update();	// calls movement handler
}

//==============================================================================

Actor*
OadPlayer( const SObjectStartupData* startupData )
{
	return new (*startupData->memory) Player(startupData);
}

//============================================================================

//==============================================================================
// camera.cc:
// Copyright (c) 1994,1995,1996,1997,1999,2000,2002,2003 World Foundry Group  
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
// Description: Camera represents a camera in the world, and uses camera movement handlers
//==============================================================================

#define _Camera_CC

#include <anim/path.hp>
#include <movement/movement.hp>
#include <oas/movement.h>
#include "camera.hp"
#include "movecam.hp"
#include "level.hp"

//==============================================================================
                            
extern MovementHandler* cameraHandlerTable[];

//==============================================================================

Camera::Camera( const SObjectStartupData* startupData ) :
	Actor( startupData ),
	_renderCamera( theLevel->GetViewPort() )
{
	assert(startupData != NULL);	// fail if no startup data passed in

	assert(ValidPtr(GetMovementBlockPtr()));
	AssertMsg( this->GetMovementBlockPtr()->Mobility == MOBILITY_CAMERA, *this << " Camera needs mobility set to 'Camera'");

//	_ObjectOnDisk* objdata = startupData->objectData;
//	Vector3 origPos( Scalar( objdata->x ), Scalar( objdata->y ), Scalar( objdata->z ) );
//	_path = NEW(PathPoint(origPos, Euler()));
//	assert( _path );
	_path = NULL;		// I want this to blow chunks if someone refers to it -- Phil

	// set camera defaults
	Color color(Color::FromInt(getOad()->FoggingColor));
	GetRenderCamera().SetFog( color, Scalar(getOad()->GetFoggingStartDistance()), Scalar(getOad()->GetFoggingCompleteDistance()) );

#if defined(DO_STEREOGRAM)
	GetRenderCamera().SetStereogram( Scalar(getOad()->EyeDistance), Angle(Angle::Degree(Scalar(getOad()->EyeAngle))) );
#endif
	// Create collision box for the Camera actor, since some handlers do collision checking
	_physicalAttributes.SetColSpace( Vector3( SCALAR_CONSTANT(-0.1), SCALAR_CONSTANT(-0.1), SCALAR_CONSTANT(-0.1) ),
									 Vector3( SCALAR_CONSTANT(0.1), SCALAR_CONSTANT(0.1), SCALAR_CONSTANT(0.1) ) );
	GetMovementManager().InitMovementHandler(*this);
}

//============================================================================

Actor::EActorKind
Camera::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Camera_KIND);
	return Actor::Camera_KIND;
}

//============================================================================

bool
Camera::CanRender() const
{
	return false;
}

bool
Camera::CanUpdate() const
{
	return true;
}

//============================================================================

const PhysicalObject*
Camera::GetWatchObject() const
{
#pragma message ("KTS: assuming camera always runs something derived from CameraMovementHandler")

   Validate();
   assert(ValidPtr(_nonStatPlat));
	assert(ValidPtr(&_nonStatPlat->_movementManager.GetMovementHandler(*this)));
	const CameraHandler* cHandler = dynamic_cast<const CameraHandler*>(&_nonStatPlat->_movementManager.GetMovementHandler(*this));
   assert(ValidPtr(cHandler));
	return cHandler->GetWatchObject(*this);
}

//============================================================================

const bool
Camera::ValidView() const
{
#pragma message ("KTS: assuming camera always runs something derived from CameraMovementHandler")
   Validate();
	assert(ValidPtr(&_nonStatPlat->_movementManager.GetMovementHandler(*this)));
	return ((CameraHandler*)&_nonStatPlat->_movementManager.GetMovementHandler(*this))->ValidView(*this);
}

//============================================================================

void
Camera::update()
{
	DBSTREAM3( ccamera << "Camera::update:" << std::endl; )
    GetMovementManager().update(*this, theLevel->GetObjectList());
	// This is a hack, because the Camera actor doesn't get run like other actors
	GetWritablePhysicalAttributes().HasRunPredictPosition(false);
}

//============================================================================
//	this constructs the camera object via the object constructor
//============================================================================

Actor*
OadCamera(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Camera( startupData );
}

//============================================================================

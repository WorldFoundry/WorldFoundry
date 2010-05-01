//============================================================================== 
// movecam.cc:
// Copyright ( c ) 1996,1997,1998,1999,2000,2001.2002,2003 World Foundry Group  
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
// Description: movement handling for the camera
//			Created	3/1/96 1:29PM KTS
//============================================================================

#define _MOVECAM_CC
#include <math/vector3.hp>
#include <movement/movement.hp>
#include <oas/movement.h>
#include <oas/camshot.ht>
#include <math/scalar.hp>
#include <math/matrix34.hp>
#include <math/euler.hp>

#include "movecam.hp"
#include "camshot.hp"
#include "actor.hp"
#include "camera.hp"

//============================================================================

DelayCameraHandler	theDelayCameraHandler;
PanCameraHandler	thePanCameraHandler;
BungeeCameraHandler	theBungeeCameraHandler;
NormalCameraHandler	theNormalCameraHandler;

extern NullHandler	theNullHandler;

extern bool gBungeeCam;

// Handlers just for "camera-like" movement
// MovementHandler* cameraHandlerTable[] =
// {
//    &theDelayCameraHandler,
//    &theBungeeCameraHandler,
//    &theNormalCameraHandler,
//    &thePanCameraHandler,
//    &theNullHandler
// };

//============================================================================
// clip a signed # to limit

template <class type> INLINE type
LimitMagnitude(const type& value, const type& limit)
{
	if(value > type(0))
		return(value.Min(limit));
	else
		return(value.Max(-limit));
}

//============================================================================
// clip a vector to a limit, on all three axis'

INLINE Vector3
LimitMagnitude(const Vector3& value, const Vector3& limit)
{
	Vector3 retVal;
	if(value.X() > Scalar::zero)
		retVal.SetX(value.X().Min(limit.X()));
	else
		retVal.SetX(value.X().Max(-limit.X()));

	if(value.Y() > Scalar::zero)
		retVal.SetY(value.Y().Min(limit.Y()));
	else
		retVal.SetY(value.Y().Max(-limit.Y()));

	if(value.Z() > Scalar::zero)
		retVal.SetZ(value.Z().Min(limit.Z()));
	else
		retVal.SetZ(value.Z().Max(-limit.Z()));

	return(retVal);
}

//============================================================================
//#if !defined (__BUNGEE_CAM__)
// this is used to implement the camera slew; it clips a vector (made from two positions)
// to a limit, and returns the new position based on the limit

INLINE Vector3
LimitRelativeMovementMagnitude(const Vector3& orig,const Vector3& outPos,const Vector3& limit)
{
	Vector3 retVal;
	retVal = orig - outPos;
	retVal = LimitMagnitude(retVal,limit);
	retVal = retVal + outPos;
	return(retVal);
}

//==============================================================================

#if defined( __PSX__ )
#if 0
#pragma message ("KTS: test code: remove this function")

void
AdjustCameraParameters(Vector3& position,Euler& rotation)
{
	u_long	padd = PadRead(1);

	padd >>= 16;
	if (padd & PADRup)
		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADRdown)
		rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADRleft)
		rotation.SetB(rotation.GetB() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADRright)
		rotation.SetB(rotation.GetB() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADR1)
		rotation.SetC(rotation.GetC() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	if (padd & PADR2)
		rotation.SetC(rotation.GetC() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));

	if (padd & PADLup)
		position.SetY(position.Y() + SCALAR_CONSTANT(0.5));
	if (padd & PADLdown)
		position.SetY(position.Y() - SCALAR_CONSTANT(0.5));
	if (padd & PADLleft)
		position.SetX(position.X() - SCALAR_CONSTANT(0.5));
	if (padd & PADLright)
		position.SetX(position.X() + SCALAR_CONSTANT(0.5));

	if (padd & PADL1)
		position.SetZ(position.Z() - SCALAR_CONSTANT(0.5));
	if (padd & PADL2)
		position.SetZ(position.Z() + SCALAR_CONSTANT(0.5));

	DBSTREAM3( cscreen << "camera position:" << std::endl << position << std::endl;
	cscreen << "camera rotation:" << std::endl << rotation << std::endl; )
}

#endif
#endif

//==============================================================================

Matrix34
LookUp
(
	const Vector3& look,
	const Vector3& up,
	const Vector3& t
)
{
	Matrix34 matrix;
	// brm hack: using BrTransformToMatrix34 for now
	// for efficiency, we should write this algorithm directly

    // LookAt To Matrix34
    //DBSTREAM3(ccamera << "WfLookAtToMatrix34: matrix = " << std::endl << *matrix << std::endl);
    //DBSTREAM3(ccamera << "WfLookAtToMatrix34: xform = " << std::endl << *xform << std::endl);

    //	1) Normalise Lookat vector to get Z component of matrix
    //	2) Cross with up vector and normalise to get X component
    //	3) Cross X & Z to get Y

    Vector3 myvz = look;
    myvz.Normalize();
    myvz = -myvz;
    Vector3 myvx = up.CrossProduct(myvz);
    myvx.Normalize();
    Vector3 myvy = myvz.CrossProduct(myvx);
#if 1
    matrix[0] = myvx;
    matrix[1] = myvy;
    matrix[2] = myvz;
    matrix[3] = t;
#else
    wf_matrix34& mat = *(wf_matrix34*)&matrix;
    br_vector3 vx,vy,vz;
    vx.v[0] = myvx.X().AsLong();
    vx.v[1] = myvx.Y().AsLong();
    vx.v[2] = myvx.Z().AsLong();
    vy.v[0] = myvy.X().AsLong();
    vy.v[1] = myvy.Y().AsLong();
    vy.v[2] = myvy.Z().AsLong();
    vz.v[0] = myvz.X().AsLong();
    vz.v[1] = myvz.Y().AsLong();
    vz.v[2] = myvz.Z().AsLong();

    mat.m[0][0] = vx.v[0]; mat.m[0][1] = vx.v[1]; mat.m[0][2] = vx.v[2];
    mat.m[1][0] = vy.v[0]; mat.m[1][1] = vy.v[1]; mat.m[1][2] = vy.v[2];
    mat.m[2][0] = vz.v[0]; mat.m[2][1] = vz.v[1]; mat.m[2][2] = vz.v[2];

    mat.m[3][0] = t.X().AsLong();
    mat.m[3][1] = t.Y().AsLong();
    mat.m[3][2] = t.Z().AsLong();
#endif
	return matrix;
}

//==============================================================================

Euler cameraEuler;            // kludge, rendcrow uses this to make scarecrows face the camera

//=============================================================================
// this allows all of the various handlers to have a single interface to the
// object's camera position and orientation, using the cameraPosition struct

void
CameraHandler::SetCamera(PhysicalObject& physicalObject, const cameraPosition& destCam)
{
#pragma message ("KTS: write field of view, hither and yon code")

	PhysicalAttributes& pa = physicalObject.GetWritablePhysicalAttributes();
	pa.SetPosition(destCam.position);

	Matrix34 mat = LookUp( destCam.direction, destCam.up, pa.Position() );
   Camera* camera = dynamic_cast<Camera*>(&physicalObject);
   assert(ValidPtr(camera));
	camera->SetCameraMatrix( mat );

	DBSTREAM3(
  		ccamera << "look = " << destCam.direction << std::endl;
  		ccamera << "up   = " <<destCam.up << std::endl;
  		ccamera << "t    = " << pa.Position() << std::endl;
  		ccamera << "lookup matrix = " << mat << std::endl;
	);

	cameraEuler.SetLookAt( destCam.direction );
	PhysicalAttributes& a = physicalObject.GetWritablePhysicalAttributes();
	a.SetRotation( cameraEuler );
}

//============================================================================
//	How camera position/orientation are calculated:
//		Position = (Camshot - follow) + TrackObject
//		Rotation = (Camshot - lookAt) + TrackObject
//============================================================================
// given a cameraShot struct, read all of the fields and fill out a cameraPosition struct accordingly

int32
SetCameraParametersFromShot(const Actor* tempcamShotActor,cameraPosition& outPos, const BaseObjectList& bol)
{
	assert(ValidPtr(tempcamShotActor));
   
	const CamShot* camShotActor = dynamic_cast<const CamShot*>(tempcamShotActor);
	assert(ValidPtr(camShotActor));

	const _CamShot* shotData = camShotActor->GetOADData();           // oad shot data for this object
	assert(shotData);
	assert(shotData->Follow);

	BaseObject* bo = bol[shotData->Follow];
   assert(ValidPtr(bo));
   PhysicalObject* po = dynamic_cast<PhysicalObject*>(bo);
   assert(ValidPtr(po));
	const Vector3& followVect = po->GetPhysicalAttributes().PredictedPosition();

	Vector3 camShotPos = camShotActor->GetPhysicalAttributes().PredictedPosition();
	Vector3 relative = camShotPos - followVect;

	// first calc position
	if(shotData->PositionX)
		outPos.position.SetX(relative.X());
	else
		outPos.position.SetX(camShotPos.X());

	if(shotData->PositionY)
		outPos.position.SetY(relative.Y());
	else
		outPos.position.SetY(camShotPos.Y());

	if(shotData->PositionZ)
		outPos.position.SetZ(relative.Z());
	else
		outPos.position.SetZ(camShotPos.Z());

//	cerror << "campos Vector is " << camPos << std::endl;
//	cerror << "follow Vector is " << followVect << std::endl;
//	cerror << "camshotactor = " << *camShotActor << std::endl;
//	cerror << "follow = " << *theLevel->getActor(shotData->Follow) << std::endl;
//	cerror << "target = " << *theLevel->getActor(shotData->Target) << std::endl;

//	cerror << "relative vector is " << relative << std::endl;
//	cerror << "resulting vector is " << outPos.position << std::endl;

//	outPos.position.SetX(Scalar(shotData->CamPositionX));
//	outPos.position.SetY(Scalar(shotData->CamPositionY));
//	outPos.position.SetZ(Scalar(shotData->CamPositionZ));

	int32 idxTrackObject;
	if(shotData->TrackObjectMailbox)
	{
		assert(shotData->TrackObjectMailbox);
		AssertMsg(camShotActor->GetMailboxes().ReadMailbox(shotData->TrackObjectMailbox).WholePart(), " in object " << *camShotActor << ", TrackObjectMailbox = " << shotData->TrackObjectMailbox);
		idxTrackObject = camShotActor->GetMailboxes().ReadMailbox(shotData->TrackObjectMailbox).WholePart();
		AssertMsg( idxTrackObject, "TrackObjectMailbox was " << shotData->TrackObjectMailbox << ", contents were " << camShotActor->GetMailboxes().ReadMailbox(shotData->TrackObjectMailbox).WholePart() );
	}
	else
	{
		AssertMsg(shotData->TrackObject, "CamShot " << *camShotActor << " doesn't have a TrackObject entry");
		AssertMsg(theLevel->GetObject(shotData->TrackObject)," Camshot is " << *camShotActor);
		idxTrackObject = shotData->TrackObject;
	}
	assert(idxTrackObject);
	PhysicalObject* trackObject = theLevel->getActor(idxTrackObject);
	assert(ValidPtr(trackObject));
	const PhysicalAttributes& trackObjectPA = trackObject->GetPhysicalAttributes();
	const Vector3& trackObjectPosition = trackObjectPA.PredictedPosition();

	// now do look at
	// kts change to calc eulers instead
//	outPos.direction.SetX(Scalar(shotData->LookAtX));
//	outPos.direction.SetY(Scalar(shotData->LookAtY));
//	outPos.direction.SetZ(Scalar(shotData->LookAtZ));

	assert(shotData->Target);
	assert(theLevel->GetObject(shotData->Target));
	Vector3 targetPos = theLevel->getActor(shotData->Target)->GetPhysicalAttributes().PredictedPosition();

//	if(shotData->Focus)
//	{			// relative
//		cerror << "relative camera focus" << std::endl;
//		outPos.direction = targetPos - followVect;
//	}
//	else
//	{			// absolute
//		cerror << "absolute camera focus" << std::endl;
		outPos.direction = targetPos - camShotPos;
//	}

// kts FIX  (Brad write code here)
	outPos.up = Vector3::unitZ;
	{
//		Euler rot( Euler::ZYX_R,
//			Angle::zero,
//			Angle::zero,
//			Revolution( theLevel->GetMailboxes().ReadMailbox( EMAILBOX_CAMROLL ) ) );
#pragma message("kts fix: put back when the euler/matrix34 problems are resolved")
#if 0
		Euler rot( Euler::ZYX_R,
			Angle::zero,
			Angle::zero,
			Angle::Revolution( theLevel->GetMailboxes().ReadMailbox( EMAILBOX_CAMROLL ) ) );
		wf_matrix34 camMatrix;

		BrEulerToMatrix34( &camMatrix, rot.BrEulerPtr() );
		outPos.up.applyMatrix34(camMatrix);
#endif
	}

	if (shotData->Rotation)	// track/mimic rotation of object
	{
// there is still a bug on the psx version where the euler constructor doesn't work
// this code is to help debug that problem, when we get around to it
//		DBSTREAM1( cnull << "track object rotations are: " <<
//			trackObjectPA.RotationA() <<
//			trackObjectPA.RotationB() <<
//			trackObjectPA.RotationC() << std::endl; )

//		printf("pa angles are: %d %d %d\n",
//			trackObjectPA.RotationA().AsBrAngle(),
//			trackObjectPA.RotationB().AsBrAngle(),
//			trackObjectPA.RotationC().AsBrAngle());

//		Euler rot( Euler::ZYX_R,
//			trackObjectPA.RotationC(),
//			Angle::Revolution(Scalar::zero),
//			Angle::Revolution(Scalar::zero) );

// kts note: Brad broke Euler::ZYX_R
//		Euler rot( Euler::XYZ_S,
		Euler rot(
			Angle::Revolution(Scalar::zero),		// ignore target roll and pitch
			Angle::Revolution(Scalar::zero),
		 	trackObjectPA.Rotation().GetC()
			);

//		printf("eulers are: %d %d %d\n",
//		rot.BrEulerPtr()->a,
//		rot.BrEulerPtr()->b,
//		rot.BrEulerPtr()->c);

//		DBSTREAM1( std::cout << "eulers are :" << rot << std::endl; )
//		DBSTREAM1( std::cout << "matrix = " << camMatrix << std::endl; )

		Matrix34 camMatrix(rot,Vector3::zero);
		outPos.position *= camMatrix;
		outPos.direction *= camMatrix;
		outPos.up *= camMatrix;
	}

	outPos.field = shotData->GetFOV();
	outPos.hither = shotData->GetHither();
	outPos.yon = shotData->GetYon();

	assert(shotData);

	if (shotData->PositionX)
		outPos.position.SetX(outPos.position.X() + trackObjectPosition.X());
	if (shotData->PositionY)
		outPos.position.SetY(outPos.position.Y() + trackObjectPosition.Y());
	if (shotData->PositionZ)
		outPos.position.SetZ(outPos.position.Z() + trackObjectPosition.Z());
//	if (shotData->Focus)
//	{
//		outPos.direction.SetX(outPos.direction.X() + trackObjectPosition.X() - outPos.position.X());
//		outPos.direction.SetY(outPos.direction.Y() + trackObjectPosition.Y() - outPos.position.Y());
//		outPos.direction.SetZ(outPos.direction.Z() + trackObjectPosition.Z() - outPos.position.Z());
//	}

	return(idxTrackObject);
}

//============================================================================

CameraHandler::cameraData&
CameraHandler::GetCameraMovementData(MovementObject& movementObject)
{
	cameraData* retVal = (cameraData*)(movementObject.GetMovementManager().GetMovementHandlerData());
   assert(ValidPtr(retVal));
	return(*retVal);
}

//============================================================================

const CameraHandler::cameraData&
CameraHandler::GetCameraMovementData(const MovementObject& movementObject) const
{
	cameraData* retVal = (cameraData*)(movementObject.GetMovementManager().GetMovementHandlerData());
   assert(ValidPtr(retVal));
	return *retVal;
}

//============================================================================
// normal camera movement

size_t
NormalCameraHandler::DataSize()
{
   return sizeof(cameraData);
}

//==============================================================================

void
NormalCameraHandler::init(MovementManager& movementManager, MovementObject& movementObject)
{
	DBSTREAM3( ccamera << "NormalCameraHandler::init() entered." << std::endl; )
	assert( ValidPtr( movementObject.GetMovementBlockPtr() ) );
	assert( movementManager.MovementBlock()->Mobility == MOBILITY_CAMERA );

	cameraData& cd  = GetCameraMovementData(movementObject);

	int shotIndex = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	AssertMsg(shotIndex != 0, "Camera " << movementObject << " found no ActBoxOR, possible cause: Player is not in any actboxor");
	assert(shotIndex > 0);

#if DO_ASSERTIONS
	{
		const CamShot* camShot = dynamic_cast<CamShot*>(theLevel->GetObject(shotIndex));
      assert(ValidPtr(camShot));
		AssertMsg(ValidPtr(camShot),"shotIndex = " << shotIndex);
		assert(camShot->kind() == BaseObject::CamShot_KIND);
	}
#endif

	cd.idxOldCamShotActor = 0;
//	cd.validView = false;
//	DBSTREAM3( ccamera << "ValidView set FALSE." << std::endl; )
	DBSTREAM3( ccamera << "NormalCameraHandler::init() exited." << std::endl; )
}

//============================================================================

bool
NormalCameraHandler::check()
{
	assert(0);
	return true;
}

//============================================================================

void
NormalCameraHandler::predictPosition(MovementManager& /*movementManager*/, MovementObject& movementObject, const Clock& /*clock*/, const BaseObjectList& /*baseObjectList*/)
{
	DBSTREAM3( ccamera << "NormalCameraHandler::predictPosition() entered." << std::endl; )

	PhysicalAttributes& actorPhysicalAttr = movementObject.GetWritablePhysicalAttributes();
	DBSTREAM3( ccamera << "NormalCameraHandler::predictPosition: physicaattributes = " << actorPhysicalAttr << std::endl; )
	actorPhysicalAttr.SetPredictedPosition(actorPhysicalAttr.Position());
	DBSTREAM3( ccamera << "NormalCameraHandler::predictPosition() exited." << std::endl; )
}

//============================================================================

bool
NormalCameraHandler::_update(MovementObject& movementObject,cameraPosition& destCam)
{
	DBSTREAM3( ccamera << "NormalCameraHandler::_update() entered." << std::endl; )

	cameraData& cd  = GetCameraMovementData(movementObject);

	long idxShot = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	DBSTREAM3( ccamera << "idxShot = " << idxShot << std::endl; )
	AssertMsg(idxShot != 0, "Camera " << movementObject << " found no ActBoxOR, possible cause: Player is not in any actboxor");
	assert(idxShot > 0);

	const CamShot* camShot = (CamShot*)theLevel->getActor(idxShot);
	assert(ValidPtr(camShot));
	assert(camShot->kind() == BaseObject::CamShot_KIND);

	cd.idxTrackObject = SetCameraParametersFromShot(camShot, destCam,theLevel->GetObjectList());				// this is the normal case, if there is nothing special going on, this one runs

//	const _CamShot* shotData = camShot->GetOADData();
//	assert(ValidPtr(shotData));

// The parameters XSLEW, YSLEW, and ZSLEW are for limiting the travel rate of the camera when
// panning between camshots.  I don't understand why they are necessary, since we specify the
// pan time in seconds in the OAD.  So, I'm defining them as constants here just to get this
// code working.  --Phil
#define XSLEW SCALAR_CONSTANT(10)
#define YSLEW SCALAR_CONSTANT(10)
#define ZSLEW SCALAR_CONSTANT(10)

	if(gBungeeCam)
	{
		cd.idxOldCamShotActor = idxShot;
		cd.oldCameraPosition = destCam.position;
		DBSTREAM3( ccamera << "NormalCameraHandler::_update() exited, returning TRUE." << std::endl; )
		return true;
	}
	else
	{
		if((cd.idxOldCamShotActor == idxShot) || cd.idxOldCamShotActor == 0)
		{
			if(cd.idxOldCamShotActor)
				destCam.position = LimitRelativeMovementMagnitude(destCam.position,cd.oldCameraPosition,Vector3(XSLEW, YSLEW, ZSLEW));
			cd.idxOldCamShotActor = idxShot;
			cd.oldCameraPosition = destCam.position;
			DBSTREAM3( ccamera << "NormalCameraHandler::_update() exited, returning TRUE." << std::endl; )
			return true;
		}
		else
		{
			cd.oldCameraPosition = destCam.position;
			movementObject.GetMovementManager().SetMovementHandler(&thePanCameraHandler,movementObject);
			cd.idxOldCamShotActor = idxShot;
			DBSTREAM3( ccamera << "NormalCameraHandler::_update() exited, returning FALSE." << std::endl; )
			return false;
		}
	}
}

//============================================================================

bool
NormalCameraHandler::update(MovementManager& /*movementManager*/,  MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
	DBSTREAM3( ccamera << "NormalCameraHandler::update() entered." << std::endl; )

	cameraData& cd = GetCameraMovementData(movementObject);
	cameraPosition destCam;
	if(_update(movementObject,destCam))
		SetCamera(movementObject,destCam);									// set camera to new settings
	cd.direction = destCam.direction;								// kts added so we can read lookat vector so we can re-map joystick
	cd.validView = true;
	DBSTREAM3( ccamera << "ValidView set TRUE." << std::endl; )
	theLevel->GetMailboxes().WriteMailbox( EMAILBOX_CAMSHOT, Scalar::zero );						// clear mailbox after use
	DBSTREAM3( ccamera << "NormalCameraHandler::update() exited." << std::endl; )
	return true;
}

//============================================================================

const PhysicalObject*
NormalCameraHandler::GetWatchObject(const MovementObject& movementObject) const
{
	DBSTREAM3( ccamera << "NormalCameraHandler::GetWatchObject() called." << std::endl; )

	const cameraData& cd = GetCameraMovementData(movementObject);
	assert(ValidPtr(&cd));
	if(cd.idxTrackObject)
	{
		BaseObject* trackObject = theLevel->GetObject(cd.idxTrackObject);
		assert(ValidPtr(trackObject));
      PhysicalObject* po = dynamic_cast<PhysicalObject*>(trackObject);
      assert(ValidPtr(po));
		return(po);
	}
	DBSTREAM1( cerror << "NormalCameraHandler::GetWatchObject: had to return mainCharacter" << std::endl; )
	return(theLevel->mainCharacter());
}

//============================================================================

bool
NormalCameraHandler::ValidView(const MovementObject& movementObject) const
{
	DBSTREAM3( ccamera << "NormalCameraHandler::ValidView() entered." << std::endl; )

	const cameraData& cd  = GetCameraMovementData(movementObject);
	assert(ValidPtr(&cd));
	return cd.validView;
}

//============================================================================

const Vector3&
NormalCameraHandler::GetLookAt(const MovementObject& movementObject) const
{
	DBSTREAM3( ccamera << "NormalCameraHandler::GetLookAt() called." << std::endl; )

	const cameraData& cd  = GetCameraMovementData(movementObject);
	assert(ValidPtr(&cd));
	return(cd.direction);
}

//============================================================================

size_t
PanCameraHandler::DataSize()
{
   return sizeof(cameraData);
}

//==============================================================================

void
PanCameraHandler::init(MovementManager& movementManager, MovementObject& movementObject)
{
	DBSTREAM3( ccamera << "PanCameraHandler::init() entered." << std::endl; )
	assert( movementManager.MovementBlock()->Mobility == MOBILITY_CAMERA );

	if(gBungeeCam)
//#if defined (__BUNGEE_CAM__)
		theBungeeCameraHandler.init(movementManager, movementObject);
//#else
	else
		NormalCameraHandler::init(movementManager, movementObject);
//#endif

	cameraData& cd  = GetCameraMovementData(movementObject);

	cd.panStartTime = theLevel->LevelClock().Current();
	cd.idxCamShotActor = cd.idxOldCamShotActor;

	DBSTREAM3( ccamera << "PanCameraHandler::init() exited." << std::endl; )
}

//============================================================================

bool
PanCameraHandler::check()
{
	assert(0);
	return true;
}

//============================================================================

void
PanCameraHandler::predictPosition(MovementManager& /*movementManager*/, MovementObject& movementObject, const Clock& /*clock*/, const BaseObjectList& /*baseObjectList*/)
{
	DBSTREAM3( ccamera << "PanCameraHandler::predictPosition() called." << std::endl; )

	PhysicalAttributes& actorPhysicalAttr = movementObject.GetWritablePhysicalAttributes();
	actorPhysicalAttr.SetPredictedPosition(actorPhysicalAttr.Position());
}

//============================================================================

bool
PanCameraHandler::update(MovementManager& /*movementManager*/,  MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
	DBSTREAM3( ccamera << "PanCameraHandler::update() entered." << std::endl; )

   char msgData[msgDataSize];

	MsgPort& msgPort = movementObject.GetMsgPort();

	// EAT ALL COLLISION MESSAGES
	while( msgPort.GetMsgByType(MsgPort::COLLISION, &msgData,msgDataSize))		// Receive a message
		;


	cameraPosition destCam;								// first let normal code do dest
	_update(movementObject,destCam);

	cameraData& cd  = GetCameraMovementData(movementObject);
	cameraPosition origCam;
	assert(cd.idxCamShotActor);
	const CamShot* origCamShotActor = (CamShot*)theLevel->getActor(cd.idxCamShotActor);
	assert(ValidPtr(origCamShotActor));
	SetCameraParametersFromShot(origCamShotActor, origCam,theLevel->GetObjectList());				// this is the Pan case, if there is nothing special going on, this one runs
//	const _CamShot* origShotData = origCamShotActor->GetOADData();

	assert(cd.panStartTime >= Scalar::zero);
	assert(cd.panStartTime <= theLevel->LevelClock().Current());
	assert(cd.idxCamShotActor);

	long shotIndex = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	assert(shotIndex);
	const CamShot* camShot = (CamShot*)theLevel->getActor(shotIndex);
	assert(ValidPtr(camShot));
	assert(ValidPtr(camShot->GetOADData()));
	Scalar panTimeInSeconds = camShot->GetOADData()->GetPanTimeInSeconds();
	assert(panTimeInSeconds > SCALAR_CONSTANT(.01));
	int index =
		 (
		 	( SCALAR_CONSTANT(100) * (theLevel->LevelClock().Current() - cd.panStartTime) )
			/ panTimeInSeconds
		 ).WholePart();
	Scalar pct(Scalar(index,0)/SCALAR_CONSTANT(100));

	// calculate the camera's expected view based on the previous camera data
	// and pan between that and the destination view
	cameraPosition panCam;
	panCam.field = ((destCam.field - origCam.field) * pct) + origCam.field;

	panCam.position.SetX(((destCam.position.X() - origCam.position.X()) * pct) + origCam.position.X());
	panCam.position.SetY(((destCam.position.Y() - origCam.position.Y()) * pct) + origCam.position.Y());
	panCam.position.SetZ(((destCam.position.Z() - origCam.position.Z()) * pct) + origCam.position.Z());

	panCam.direction.SetX(((destCam.direction.X() - origCam.direction.X()) * pct) + origCam.direction.X());
	panCam.direction.SetY(((destCam.direction.Y() - origCam.direction.Y()) * pct) + origCam.direction.Y());
	panCam.direction.SetZ(((destCam.direction.Z() - origCam.direction.Z()) * pct) + origCam.direction.Z());

	panCam.up.SetX(((destCam.up.X() - origCam.up.X()) * pct) + origCam.up.X());
	panCam.up.SetY(((destCam.up.Y() - origCam.up.Y()) * pct) + origCam.up.Y());
	panCam.up.SetZ(((destCam.up.Z() - origCam.up.Z()) * pct) + origCam.up.Z());

	panCam.field = Scalar(((destCam.field - origCam.field) * pct) + origCam.field);

	panCam.hither = Scalar(((destCam.hither - origCam.hither) * pct) + origCam.hither);
	panCam.yon = Scalar(((destCam.yon - origCam.yon) * pct) + origCam.yon);

	SetCamera(movementObject,panCam);									// set camera to new settings

	if( theLevel->LevelClock().Current() > cd.panStartTime + panTimeInSeconds )
	{
  		if(gBungeeCam)
//#if defined (__BUNGEE_CAM__)
			movementObject.GetMovementManager().SetMovementHandler(&theBungeeCameraHandler,movementObject);
		else
//#else
			movementObject.GetMovementManager().SetMovementHandler(&theNormalCameraHandler,movementObject);
//#endif
		DBSTREAM3( ccamera << "PanCameraHandler::update() exited, returning FALSE." << std::endl; )

		return false;
	}

	DBSTREAM3( ccamera << "PanCameraHandler::update() exited, returning TRUE." << std::endl; )

	return true;
}

//============================================================================

const PhysicalObject*
PanCameraHandler::GetWatchObject(const MovementObject& movementObject) const
{
	DBSTREAM3( ccamera << "PanCameraHandler::GetWatchObject() called." << std::endl; )

	const cameraData& cd  = GetCameraMovementData(movementObject);
	assert(ValidPtr(&cd));
	if(cd.idxTrackObject)
	{
		BaseObject* trackObject = theLevel->GetObject(cd.idxTrackObject);
		assert(ValidPtr(trackObject));
      PhysicalObject* po = dynamic_cast<PhysicalObject*>(trackObject);
      assert(ValidPtr(po));
		return(po);
	}
//	assert(0);
	DBSTREAM1( cerror << "PanCameraHandler::GetWatchObject: had to return mainCharacter" << std::endl; )
	return(theLevel->mainCharacter());
}

//============================================================================

const Vector3&
PanCameraHandler::GetLookAt(const MovementObject& movementObject) const
{
	DBSTREAM3( ccamera << "PanCameraHandler::GetLookAt() called." << std::endl; )

	const cameraData& cd  = GetCameraMovementData(movementObject);
	return(cd.direction);
}

//============================================================================
// local functions

//============================================================================

size_t
DelayCameraHandler::DataSize()
{
   return sizeof(cameraData);
}

//==============================================================================

void
DelayCameraHandler::init(MovementManager& movementManager, MovementObject& movementObject)
{
	DBSTREAM3( ccamera << "DelayCameraHandler selected." << std::endl; )
	assert( movementManager.MovementBlock()->Mobility == MOBILITY_CAMERA );

	cameraData& cd  = GetCameraMovementData(movementObject);
	cd.delayCounter = 0;

//	DBSTREAM1( cerror << "DelayCameraHandler::init(): placing camera at mainCharacter position" << std::endl; )
//	Actor* theMainCharacter = theLevel->mainCharacter();
//	assert( theMainCharacter );
//  cameraPosition destCam;
//	destCam.position = theMainCharacter->currentPos();
// 	SetCamera(movementObject, destCam);	// Set a valid position initially
}

//============================================================================

bool
DelayCameraHandler::check()
{
	assert(0);
	DBSTREAM3( ccamera << "DelayCameraHandler check function called." << std::endl; )
	long shotIndex = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	return(shotIndex <= 0);						// only activate if there is no camshot right now
//	return true;
}

//============================================================================

void
DelayCameraHandler::predictPosition(MovementManager& /*movementManager*/, MovementObject& movementObject, const Clock& /*clock*/, const BaseObjectList& /*baseObjectList*/)
{
	DBSTREAM3( ccamera << "DelayCameraHandler::predictPosition function called." << std::endl; )
	PhysicalAttributes& actorPhysicalAttr = movementObject.GetWritablePhysicalAttributes();
	actorPhysicalAttr.SetPredictedPosition(actorPhysicalAttr.Position());
}

//============================================================================

bool
DelayCameraHandler::update(MovementManager& movementManager, MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
	DBSTREAM3( ccamera << "DelayCameraHandler::update function called." << std::endl; )
	assert( ValidPtr( theLevel ) );

	long shotIndex = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	//std::cout << "shotIndex = " << shotIndex << std::endl;
	if(shotIndex > 0)
	{
	if(gBungeeCam)
//#if defined (__BUNGEE_CAM__)
		movementManager.SetMovementHandler(&theBungeeCameraHandler,movementObject);
	else
//#else
		movementManager.SetMovementHandler(&theNormalCameraHandler,movementObject);
//#endif
		return false;
	}

	cameraData& cd  = GetCameraMovementData(movementObject);
	DBSTREAM3( ccamera << "  cd.delayCounter = " << cd.delayCounter << std::endl; )
	cd.delayCounter++;
	AssertMsg(cd.delayCounter < 5, "Camera " << movementObject << " found no ActBoxOR, possible cause: Player is not in any actboxor");
	return true;
}

//============================================================================

const PhysicalObject*
DelayCameraHandler::GetWatchObject(const MovementObject& ) const
{
	return(theLevel->mainCharacter());
}

//============================================================================

bool
DelayCameraHandler::ValidView(const MovementObject&  ) const
{
	return false;
}

//============================================================================

const Vector3&
DelayCameraHandler::GetLookAt(const MovementObject& ) const
{
	return(Vector3::zero);
}

//============================================================================

size_t
BungeeCameraHandler::DataSize()
{
   return sizeof(cameraData);
}

//==============================================================================

void
BungeeCameraHandler::init(MovementManager& movementManager, MovementObject& movementObject)
{
	DBSTREAM3( ccamera << "BungeeCam::init() entered." << std::endl; )
	assert( ValidPtr( movementObject.GetMovementBlockPtr() ) );
	assert( movementManager.MovementBlock()->Mobility  == MOBILITY_CAMERA );

	cameraData& cd = GetCameraMovementData(movementObject);

	long shotIndex = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	AssertMsg(shotIndex != 0, "Camera " << movementObject << " found no ActBoxOR, possible cause: Player is not in any actboxor");
	assert(shotIndex > 0);

#if DO_ASSERTIONS
	{
		const CamShot* camShot = (CamShot*)theLevel->getActor(shotIndex);
		AssertMsg(ValidPtr(camShot),"shotIndex = " << shotIndex);
		AssertMsg( camShot->kind() == Actor::CamShot_KIND, "camShot->kind() = " << camShot->kind() );
	}
#endif

	cd.idxOldCamShotActor = 0;
	cd.validView = false;
	DBSTREAM3( ccamera << "ValidView set FALSE." << std::endl; )

//   	cameraPosition destCam;
//   	_update(movementObject, destCam);	// Fill in destCam with the "desired" new position/orientation
// 	SetCamera(movementObject, destCam);	// Set a valid position initially
	DBSTREAM3( ccamera << "BungeeCam::init() exited." << std::endl; )
}

//============================================================================

bool
BungeeCameraHandler::check()
{
	assert(0);
	return true;
}

//============================================================================

void
BungeeCameraHandler::predictPosition(MovementManager& /*movementManager*/, MovementObject& movementObject, const Clock& /*clock*/, const BaseObjectList& /*baseObjectList*/)
{
	DBSTREAM3( ccamera << "BungeeCam::predictPosition() entered." << std::endl; )

	assert( movementObject.kind() == Actor::Camera_KIND );

	cameraData& cd  = GetCameraMovementData(movementObject);
	Scalar deltaT = theLevel->LevelClock().Delta();

	// Get climbRate and Elasticity from camShot data
	long idxShot = theLevel->GetMailboxes().ReadMailbox(EMAILBOX_CAMSHOT).WholePart();
	RangeCheck( 1, idxShot, theLevel->GetMaxObjectIndex() );

	CamShot* camShotActor = (CamShot*)(theLevel->getActor( idxShot ));
	assert( ValidPtr( camShotActor ) );
	const _CamShot* shotData = camShotActor->GetOADData();
	assert( ValidPtr( shotData ) );

   	cameraPosition destCam;
   	_update(movementObject, destCam);	// Fill in destCam with the "desired" new position/orientation
	DBSTREAM3( ccamera << "Desired camera position: " << destCam.position << std::endl; )

	theLevel->GetMailboxes().WriteMailbox( EMAILBOX_CAMSHOT, Scalar::zero );					// clear mailbox after use
	PhysicalAttributes& actorAttr = movementObject.GetWritablePhysicalAttributes();
	Vector3 origLinVelocity = actorAttr.LinVelocity();

	// check collision messages
   char msgData[msgDataSize];

	MsgPort& msgPort = movementObject.GetMsgPort();
	bool hitSomething=false, hitTop=false;

	while( msgPort.GetMsgByType( MsgPort::SPECIAL_COLLISION, &msgData, msgDataSize ) )
	{
		hitSomething = true;
      Actor* actor = (Actor*)&msgData;
      assert(ValidPtr(actor));

		if (movementObject.GetPhysicalAttributes().Position().Z() < actor->GetPhysicalAttributes().Position().Z())
			hitTop = true;
	}

	// if collision prevention may make us climb, don't seek the target in Z
	if (hitSomething)
		destCam.position.SetZ( actorAttr.Position().Z() );

	actorAttr.SetLinVelocity( (destCam.position - actorAttr.Position()) / (deltaT * shotData->GetElasticity()) );

	DBSTREAM3( ccamera << "Desired range to target: " << destCam.direction.Length() << std::endl <<
						  " Actual range to target: " << cd.direction.Length() << std::endl; )

	// Move upwards to avoid obstacle unless collision is on top side of camera
	if (hitSomething && !hitTop)
	{
		// Stop moving up if it will make us exceed the camera-target radius called for by the camshot
		if ( (cd.direction.Z().Abs() < destCam.direction.Length()) )
			actorAttr.AddLinVelocity( Vector3(Scalar::zero, Scalar::zero, shotData->GetClimbRate()) );
	}

   Camera* camera = dynamic_cast<Camera*>(&movementObject);
   assert(ValidPtr(camera));
	camera->cameraPos = destCam;	// store this data for use in update()

   Vector3 newPosition(actorAttr.Position() + (((actorAttr.LinVelocity() + origLinVelocity) / Scalar::two) * theLevel->LevelClock().Delta())); 
   DBSTREAM1( ccamera << "BungeeCamera:predictPosition: actor physicalAttributes = " << actorAttr << ", new position: " << newPosition << std::endl; )
	actorAttr.SetPredictedPosition(newPosition);
}

//============================================================================

bool
BungeeCameraHandler::update(MovementManager& /*movementManager*/, MovementObject& movementObject, const BaseObjectList& baseObjectList )
{
	DBSTREAM3( ccamera << "BungeeCam::update() entered." << std::endl; )

	cameraData& cd  = GetCameraMovementData(movementObject);
   Camera* camera = dynamic_cast<Camera*>(&movementObject);
   assert(ValidPtr(camera));
	cameraPosition destCam = camera->cameraPos;

	// Do collision check loop and don't allow motion if there's something in the way
	PhysicalAttributes& actorAttr = movementObject.GetWritablePhysicalAttributes();
	cd.collisionDirections = 0L;

	//actorAttr.SetPosition(actorAttr.PredictedPosition());
   actorAttr.Update();
	destCam.position = actorAttr.Position();

	// set look-at vector to point at target (after PreventCollision may have changed our position)
	CamShot* camShotActor = (CamShot*)(theLevel->getActor( cd.idxOldCamShotActor ));
	const _CamShot* shotData = camShotActor->GetOADData();
	assert(shotData);
	assert(shotData->Target);
	assert(theLevel->getActor(shotData->Target));
	DBSTREAM3( ccamera << "Target actor = " << *theLevel->getActor(shotData->Target) << std::endl; )
	Vector3 targetPos = (theLevel->getActor(shotData->Target))->GetPredictedPosition();
	DBSTREAM3( ccamera << "Target actor position = " << targetPos << std::endl; )
	targetPos -= (theLevel->getActor(shotData->Follow))->GetPredictedPosition();
	DBSTREAM3( ccamera << "Follow actor = " << *theLevel->getActor(shotData->Follow) << std::endl; )
	DBSTREAM3( ccamera << "Follow actor position = " << (theLevel->getActor(shotData->Follow))->GetPredictedPosition() << std::endl; )
	targetPos += (theLevel->getActor(shotData->TrackObject))->GetPredictedPosition();
	DBSTREAM3( ccamera << "TrackObject actor = " << *theLevel->getActor(shotData->TrackObject) << std::endl; )
	DBSTREAM3( ccamera << "TrackObject position = " << (theLevel->getActor(shotData->TrackObject))->GetPredictedPosition() << std::endl; )
	DBSTREAM3( ccamera << "Modified target position = " << targetPos << std::endl; )
	destCam.direction = targetPos - destCam.position;

	DBSTREAM3( ccamera << "Setting new camera position: " << destCam.position << std::endl; )
	SetCamera(movementObject, destCam);

	DBSTREAM3( ccamera << "New camera look-at vector: " << destCam.direction << std::endl; )
	cd.direction = destCam.direction;							// kts added so we can read lookat vector so we can re-map joystick
	cd.validView = true;
	DBSTREAM3( ccamera << "ValidView set TRUE." << std::endl; )

	DBSTREAM3( ccamera << "BungeeCam::update() exited.  --- END OF FRAME ---" << std::endl << std::endl; )
	return true;
}

//============================================================================

const PhysicalObject*
BungeeCameraHandler::GetWatchObject(const MovementObject& movementObject) const
{
//	DBSTREAM3( ccamera << "BungeeCam::GetWatchObject() called." << std::endl; )

	const cameraData& cd  = GetCameraMovementData(movementObject);
	assert(ValidPtr(&cd));
	if(cd.idxTrackObject)
	{
		BaseObject* trackObject = theLevel->GetObject(cd.idxTrackObject);
		assert(ValidPtr(trackObject));
      PhysicalObject* po = dynamic_cast<PhysicalObject*>(trackObject);
      assert(ValidPtr(po));
		return(po);
	}
	DBSTREAM1( cerror << "BungeeCameraHandler::GetWatchObject: had to return mainCharacter" << std::endl; )
	return(theLevel->mainCharacter());
}

//============================================================================

const Vector3&
BungeeCameraHandler::GetLookAt(const MovementObject& movementObject) const
{
//	DBSTREAM3( ccamera << "BungeeCam::GetLookAt() called." << std::endl; )
	const cameraData& cd  = GetCameraMovementData(movementObject);
	return(cd.direction);
}

//============================================================================

//============================================================================
// Light.cc:
// Copyright (c) 1997,1999,2000,2002,2003 World Foundry Group.  
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
//
//	Abstract:  Actor subclass Light (directional and ambient lighting)
//	History:
//			Created	6/2/97 16:53 by Phil Torre
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
#include "light.hp"
#include "oas/light.ht"		// get oad structure information

//============================================================================

Light::Light(const SObjectStartupData* startupData)
	: Actor(startupData)
{
	const _Light* myOAD = getOad();

//	int32 myType = myOAD->lightType;

	// Color values are 16.16 numbers from 0.0 to 1.0
	Scalar myRedValue = myOAD->GetlightRed();
	Scalar myGreenValue = myOAD->GetlightGreen();
	Scalar myBlueValue = myOAD->GetlightBlue();
	_color = Color(		uint((myRedValue   * SCALAR_CONSTANT(255)).WholePart() & 0xff),
						uint((myGreenValue * SCALAR_CONSTANT(255)).WholePart() & 0xff),
						uint((myBlueValue  * SCALAR_CONSTANT(255)).WholePart() & 0xff)
						);

//	Vector3 myVector(myOAD->lightX, myOAD->lightY, myOAD->lightZ);

	if(myOAD->lightType == DIRECTIONAL_LIGHT)
	{
//		std::cout << "making directional light" << std::endl;
//		std::cout << "rgb: " << myRedValue << ',' << myGreenValue << ',' << myBlueValue << std::endl;

	}
	else
	{
//		std::cout << "ambient light constructed" << std::endl;
//		assert(0);
	}

#pragma message ("KTS: remove after making actor set these (currently no work if type set to box")
	const _ObjectOnDisk* objdata = startupData->objectData;
   Euler euler( Angle::Revolution( Scalar( 0,objdata->rotation.a )),Angle::Revolution( Scalar( 0,objdata->rotation.b ) ),Angle::Revolution( Scalar( 0,objdata->rotation.c ) ));
//    _physicalAttributes.SetRotationA( Angle::Revolution( Scalar( 0,objdata->rotation.a ) ) ); // Set initial rotation
//    _physicalAttributes.SetRotationB( Angle::Revolution( Scalar( 0,objdata->rotation.b ) ) ); // Set initial rotation
//    _physicalAttributes.SetRotationC( Angle::Revolution( Scalar( 0,objdata->rotation.c ) ) ); // Set initial rotation
    _physicalAttributes.SetRotation( euler );
}

//============================================================================

Actor::EActorKind
Light::kind() const
{
   assert(GetMovementBlockPtr()->MovementClass == Actor::Light_KIND);
	return Actor::Light_KIND;
}

void
Light::initPath( QInputDigital* )
{
//	Actor::initPath(level);
}

//============================================================================

bool
Light::CanRender() const
{
	return true;
}

//=============================================================================

Actor*
OadLight(const SObjectStartupData* startupData)
{
	return new (*startupData->memory) Light(startupData);
}

//============================================================================


//==============================================================================
// oas/movement.h:
// Copyright ( c ) 2002 World Foundry Group  
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
// Description: hand maintained enumertion of movemnt types
// Original Author: Kevin T. Seghetti
//==============================================================================

#ifndef OAS_MOVEMENT_H
#define OAS_MOVEMENT_H

//==============================================================================

#pragma message("Is there an automatic way to create this enum from oas?")
// Keep this enumeration in sync with the "Mobility" field from movebloc.inc
enum
{
    MOBILITY_ANCHORED=0,
	MOBILITY_PHYSICS,
	MOBILITY_PATH,
	MOBILITY_CAMERA,
	MOBILITY_FOLLOW,
	MOBILITY_MAX
};

//==============================================================================
#endif
//==============================================================================


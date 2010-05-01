// ===========================================================================
// Copyright (c) 1994,95,96,97,99 World Foundry Group  
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

// ===========================================================================
// Description: The version of the pigsys.h subsystem.
// Original Author: Andre Burgoyne
// ===========================================================================

#ifndef	_PSYSVERS_H
#define	_PSYSVERS_H

// ===========================================================================
// Included from pigsys.h

// PSYS_VERSION_BODY is what should be edited when making a new version of the subsystem
#define __PIGS__                0x01000603

#define	PSYS_VERSION_BODY	"v1.0.6d3"

#define	PSYS_VERSION_HEAD	"pigsys.h: "

#define	PSYS_VERSION		PSYS_VERSION_HEAD PSYS_VERSION_BODY

#ifdef	__cplusplus
extern "C" {
#endif	//__cplusplus

	// this could go away on FINAL_RELEASE to save a little space
#if		defined(DESIGNER_CHEATS)
extern char	pigsys_version[];
#endif

#ifdef	__cplusplus
}
#endif	//__cplusplus

// ===========================================================================
#endif	//!defined(_PSYSVERS_H)
// ===========================================================================

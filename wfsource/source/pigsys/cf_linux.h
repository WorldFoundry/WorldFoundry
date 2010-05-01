//=============================================================================
// cf_linux.h:
// Copyright (c) 1994,95,96,97,98,99 World Foundry Group  
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
// Description: Generic configuration file compiling for PSX.
// Original Author: Kevin T. Seghetti
// ===========================================================================
//=============================================================================

#ifndef	_CF_LINUX_H
#define	_CF_LINUX_H

#if defined( __cplusplus )
const MachineType gHostMachineType = LINUX;
#endif

#define	WF_BIG_ENDIAN		0
#define _BOOL_IS_DEFINED_ 1

#ifndef __LINUX__
#define __LINUX__
#endif

// GNU doesn't have this yet...
//#define explicit

	// is short access is efficient on this architecture?
//#define	SYS_SMALLINT	short

#include <stddef.h>

#define _MAX_PATH PATH_MAX
#define DIRECTORY_SEPARATOR '/'

void
strlwr( char* string );

void
_linux_init(void);

	// These are not defined in the header files.
START_EXTERN_C
#include <stdio.h>
END_EXTERN_C

//=============================================================================
#endif	//!defined(_CF_PSX_H)
//=============================================================================

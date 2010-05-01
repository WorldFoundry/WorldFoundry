//=============================================================================
// iostream.h:
// Copyright ( c ) 1995,96,97,99 World Foundry Group  
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
// Description:
//		Here is my first stab at a simple ostream (kts)
//
// Original Author: Brad McKee
// Since then: Kevin T. Seghetti
// ------------------------------------------------------------------------

#ifndef _IOSTREAM_H
#define _IOSTREAM_H

#if DO_IOSTREAMS
#include <pigsys/pigsys.hp>

// ------------------------------------------------------------------------
#include "ios.h"
#include "istream.h"
#include "ostream.h"
#include "fstream.h"
// ------------------------------------------------------------------------
#include "iostream.hpi"
// ------------------------------------------------------------------------
#endif	// DO_IOSTREAMS
#endif // _IOSTREAM_H
// ------------------------------------------------------------------------

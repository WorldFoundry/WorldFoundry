//=============================================================================
// util.cc: platform specific debugging utility functions
// Copyright ( c ) 1997,1998,1999,2000 World Foundry Group  
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
// Description: The viewport class encapsulates data and behavior for a single
//	 hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#if defined ( __PSX__ )
#	include <gfx/psx/gteutil.cc>
#elif defined ( __WIN__ )
#elif defined ( __LINUX__ )
#else
#	error no util functions for this platform
#endif

//============================================================================

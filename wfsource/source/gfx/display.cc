//=============================================================================
// display.cc: display hardware abstraction class
// Copyright ( c ) 1997,1998,1999,2000,2001 World Foundry Group  
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
// Description: The Display class encapsulates data and behavior for a single
//	 hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#include <gfx/display.hp>

#if defined ( RENDERER_PSX )
#include <gfx/psx/display.cc>

#elif defined ( RENDERER_GL)
#include <gfx/gl/display.cc>
#pragma comment( lib, "opengl32.lib" )

#elif defined ( RENDERER_XWINDOWS)
#include <gfx/xwindows/display.cc>

#elif defined ( RENDERER_DIRECTX)
#include <gfx/directx/display.cc>
#pragma comment( lib, "ddraw.lib" )
#pragma comment( lib, "dxguid.lib" )

#else
#error no platform specific display code!

#endif

//============================================================================

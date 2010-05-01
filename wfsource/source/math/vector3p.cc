//=============================================================================
// math/Vector3P.cc: (Vector3PS) Platform specific Vectors
// Copyright (c) 1997,98,99 World Foundry Group  
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

// Orignal Author: Kevin T. Seghetti
//=============================================================================

#include <pigsys/pigsys.hp>
#include <math/vector3p.hp>

//=============================================================================

#if defined(__PSX__) && !defined(RENDERER_PIPELINE_SOFTWARE)
#include "psx/Vector3p.cc"

#else

#endif

//=============================================================================

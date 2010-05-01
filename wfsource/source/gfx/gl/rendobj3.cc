//=============================================================================
// gfx/gl/rendobj3.cc: 3D renderable object, psx specific code
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
// Description:
//
// Original Author: Kevin T. Seghetti
//============================================================================
// included from gfx/rendobj3

#include <math/vector3.hp>
#include <math/matrix34.hp>
#include <gfx/math.hp>

//============================================================================
// kts maybe I should write an SVECTOR class

INLINE void
ConvertVectorToVector3_PS(const Vector3& source,Vector3_PS& dest)
{
	dest = source;
}

//=============================================================================

RendererVariables globalRendererVariables;
//Matrix34 globalGTEMatrix;
//const TriFace* currentRenderFace;
//const Material* currentRenderMaterial;

void
RenderObject3D::Render(ViewPort& vp,const Matrix34& position)
{
#pragma message ("KTS " __FILE__ ": added gl render code")
}

//============================================================================

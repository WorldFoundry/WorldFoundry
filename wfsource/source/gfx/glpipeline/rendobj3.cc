//=============================================================================
// gfx/software/rendobj3.cc: 3D renderable object, psx specific code
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
// Description:
//
// Original Author: Kevin T. Seghetti
//============================================================================
// included from gfx/rendobj3

#include <cpplib/stdstrm.hp>
#include <math/vector3.hp>
#include <math/matrix34.hp>
#include <gfx/prim.hp>
#include <gfx/material.hp>
#include <gfx/math.hp>
#include <gfx/display.hp>

//============================================================================
// kts maybe I should write an SVECTOR class

INLINE void
ConvertVectorToVector3_PS(const Vector3& source,Vector3_PS& dest)
{
	dest = source;
}

//=============================================================================

RendererVariables globalRendererVariables;

void
RenderObject3D::Render(ViewPort& vp,const Matrix34& position)
{
	globalRendererVariables.viewPort = &vp;
//	cout << "RenderObject3D::Render: " << std::endl;

   Matrix34 inverted = Matrix34
   (
      Vector3( position[0][0],-position[0][1],-position[0][2]),
      Vector3( position[1][0],-position[1][1],-position[1][2]),
      Vector3( position[2][0],-position[2][1],-position[2][2]),
      Vector3( position[3][0],-position[3][1],-position[3][2])
   );
  globalRendererVariables.GTEMatrix = inverted;

  glMatrixMode( GL_MODELVIEW );
  LoadGLMatrixFromMatrix34(position);

	Primitive* primitive  = _primList[0];
	pRenderObj3DFunc renderer;

	globalRendererVariables.currentRenderFace = _faceList;
	for(int faceIndex=0;faceIndex<_faceCount;)
	{
		DBSTREAM3( cdebug << "Drawing face " << faceIndex << std::endl; )

		assert(globalRendererVariables.currentRenderFace->materialIndex >= 0);
		assert(globalRendererVariables.currentRenderFace->materialIndex < 100);  // kts temp
		globalRendererVariables.currentRenderMaterial = &_materialList[globalRendererVariables.currentRenderFace->materialIndex];
		assert( globalRendererVariables.currentRenderMaterial );
		globalRendererVariables.currentRenderMaterial->Validate();
		renderer = globalRendererVariables.currentRenderMaterial->Get3DRenderer();
		int currentMaterial = globalRendererVariables.currentRenderFace->materialIndex;

		while(currentMaterial == globalRendererVariables.currentRenderFace->materialIndex && faceIndex<_faceCount)
		{
			globalRendererVariables.gteVect[0] = Vector3ToPS(_vertexList[globalRendererVariables.currentRenderFace->v1Index].position);
			globalRendererVariables.gteVect[1] = Vector3ToPS(_vertexList[globalRendererVariables.currentRenderFace->v2Index].position);
			globalRendererVariables.gteVect[2] = Vector3ToPS(_vertexList[globalRendererVariables.currentRenderFace->v3Index].position);

			assert( ValidPtr(globalRendererVariables.currentRenderFace) );
			//globalRendererVariables.currentRenderFace->Validate();

			assert(ValidPtr(this));
	#if defined(__WIN__)
			long* rendererPtr = (long*)&renderer;
	//		cout << "rendererPtr = " << rendererPtr << std::endl;
	//		cout << "renderer = " << hex << *rendererPtr << std::endl;
			assert(ValidCodePtr((void*)(*rendererPtr)));
	#endif
	//		cout << "RenderObject3D::Render: calling renderer " << std::endl;
			(this->*renderer)(primitive);
	//		cout << "RenderObject3D::Render: done calling renderer " << std::endl;
			primitive++;
			faceIndex++;
			globalRendererVariables.currentRenderFace++;
		}
	}
	assert(_faceList[_faceCount].materialIndex = -1);
//	cout << "RenderObject3D::Render: done" << std::endl;
}

//============================================================================

//=============================================================================
// gfx/software/rendobj3.cc: 3D renderable object, psx specific code
// Copyright ( c ) 1997,98,99 World Foundry Group  
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

//#include <cpplib/stdstrm.hp>
#include <math/vector3.hp>
#include <math/matrix34.hp>
//#include <gfx/prim.hp>
//#include <gfx/material.hp>
#include <gfx/math.hp>
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <gfx/directx/winmain.hp>
#include <gfx/directx/winproc.hp>
#include <gfx/directx/d3dtex.hp>
#include <gfx/directx/scene.hp>

//============================================================================
// kts maybe I should write an SVECTOR class

INLINE void
ConvertVectorToVector3_PS(const Vector3& source,Vector3_PS& dest)
{
	dest = source;
}

inline D3DMATRIX
IdentityMatrix()
{
	return D3DMATRIX(1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1);
}


inline D3DMATRIX
Translate(const float dx, const float dy, const float dz)
{
	D3DMATRIX ret = IdentityMatrix();
	ret(3, 0) = dx;
	ret(3, 1) = dy;
	ret(3, 2) = dz;
	return ret;
} // end of Translate()


RendererVariables globalRendererVariables;
//Matrix34 globalGTEMatrix;
//const TriFace* currentRenderFace;
//const Material* currentRenderMaterial;


void
RenderObject3D::Render(ViewPort& vp,const Matrix34& position)
{

	// Get D3D window pointer
    LPD3DWindow lpd3dWindow =  (LPD3DWindow)GetWindowLong (g_hMainWindow, GWL_USERDATA);
	ValidatePtr(lpd3dWindow);

	LPDIRECT3DDEVICE2 lpd3dDevice = lpd3dWindow->GetD3DDevice();
	ValidatePtr(lpd3dDevice);

	D3DTEXTUREHANDLE	d3dTextureHandle = videoMemoryTexture.GetHandle();

//	cout << "mat34\n" << position << endl;
	D3DMATRIX objectPosition;
	Matrix34ToD3DMATRIX(objectPosition,position);
//	cout << "op:\n" << objectPosition << endl;
//	objectPosition = Translate(0,0,-6);
//	cout << "fake:\n" << objectPosition << endl;

	lpd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &objectPosition);

	// loop through object, drawing polys
	const TriFace* currentRenderFace;
	const Material* currentRenderMaterial;

	currentRenderFace = _faceList;
	for(int faceIndex=0;faceIndex<_faceCount;)
	{
		DBSTREAM3( cdebug << "Drawing face " << faceIndex << endl; )

		RangeCheck(0,currentRenderFace->materialIndex,100);  // kts arbitrary
		currentRenderMaterial = &_materialList[currentRenderFace->materialIndex];
		assert( currentRenderMaterial );
		currentRenderMaterial->Validate();
		int currentMaterial = currentRenderFace->materialIndex;

		if(currentRenderMaterial->GetMaterialFlags() & Material::TEXTURE_MAPPED)
			lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, d3dTextureHandle);

		while(currentMaterial == currentRenderFace->materialIndex && faceIndex<_faceCount)
		{
			assert( ValidPtr(currentRenderFace) );
			//currentRenderFace->Validate();

			long color = currentRenderMaterial->GetColor().AsLong();

#if 0
			D3DVECTOR normal = Vector3ToD3DVECTOR(currentRenderFace->normal);
			D3DVERTEX vertexList[3];
			vertexList[0] = D3DVERTEX(
				Vector3ToD3DVECTOR(_vertexList[currentRenderFace->v1Index].position),
				normal,currentRenderFace->_u[0],currentRenderFace->_v[0] );

			vertexList[1] = D3DVERTEX(
				Vector3ToD3DVECTOR(_vertexList[currentRenderFace->v2Index].position),
				normal,currentRenderFace->_u[1],currentRenderFace->_v[1] );

			vertexList[2] = D3DVERTEX(
				Vector3ToD3DVECTOR(_vertexList[currentRenderFace->v3Index].position),
				normal,currentRenderFace->_u[2],currentRenderFace->_v[2] );

			lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_VERTEX ,(LPVOID)vertexList,3,NULL);
#else
			D3DLVERTEX vertexList[3];
			vertexList[0] = D3DLVERTEX(
				Vector3ToD3DVECTOR(_vertexList[currentRenderFace->v1Index].position),
				color,D3DRGB(1,0,0),currentRenderFace->_u[0],currentRenderFace->_v[0] );

			vertexList[1] = D3DLVERTEX(
				Vector3ToD3DVECTOR(_vertexList[currentRenderFace->v2Index].position),
				color,D3DRGB(1,0,0),currentRenderFace->_u[1],currentRenderFace->_v[1] );

			vertexList[2] = D3DLVERTEX(
				Vector3ToD3DVECTOR(_vertexList[currentRenderFace->v3Index].position),
				color,D3DRGB(1,0,0),currentRenderFace->_u[2],currentRenderFace->_v[2] );

			lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_LVERTEX,(LPVOID)vertexList,3,NULL);
#endif
			assert(ValidPtr(this));
			faceIndex++;
			currentRenderFace++;
		}
		lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
	}
	assert(_faceList[_faceCount].materialIndex = -1);
//	cout << "RenderObject3D::Render: done" << endl;
}

//============================================================================

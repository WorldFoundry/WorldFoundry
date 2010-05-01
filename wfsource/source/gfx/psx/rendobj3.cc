//=============================================================================
// gfx/psx/rendobj3.cc: 3D renderable object, psx specific code
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

#include <gfx/psx/rendobj3.hp>
#include <math/matrixps.hp>
#include <cpplib/stdstrm.hp>
#include <math/vector3.hp>
#include <math/matrix34.hp>
#include <hal/hal.h>
#include <gfx/prim.hp>
#include <gfx/psx/gteflags.h>
#include <gfx/material.hp>
#include <gfx/math.hp>
#include <gfx/camera.hp>

//============================================================================
// kts maybe I should write an SVECTOR class

INLINE void
ConvertVectorToVector3_PS(const Vector3& source,Vector3_PS& dest)
{
	dest.SetX(source.X().AsLong() >> 8);
	dest.SetY(source.Y().AsLong() >> 8);
	dest.SetZ(source.Z().AsLong() >> 8);
}

//=============================================================================

#if DO_IOSTREAMS
INLINE ostream&
operator<<(ostream& out, const SVECTOR& vect)
{
	out << vect.vx << "," << vect.vy << "," << vect.vz;
	return(out);
}
#endif

//=============================================================================

void
RenderObject3D::Render(ViewPort& viewPort,const Matrix34& position)
{
	viewPort.Validate();
	Matrix34 mangled = Matrix34
	(
		Vector3( position[0][0], position[1][0], position[2][0]),
		Vector3(-position[0][1],-position[1][1],-position[2][1]),
		Vector3(-position[0][2],-position[1][2],-position[2][2]),
		Vector3( position[3][0],-position[3][1],-position[3][2])
	);
	Matrix34_PS gteMatrix(mangled);
	gteMatrix.SetGTE();

		viewPort.Validate();
#if DO_ASSERTIONS
		register RendererScratchVariablesStruct* scratchVariables = (RendererScratchVariablesStruct*)getScratchAddr(12);			// assume allocated from above
#else
		register RendererScratchVariablesStruct* scratchVariables = (RendererScratchVariablesStruct*)getScratchAddr(0);			// assume allocated from above
#endif
		assert(ValidPtr(scratchVariables));
		scratchVariables->rv.viewPort = &viewPort;
		assert(ValidPtr(scratchVariables->rv.viewPort));

		register RotatedVector* rotatedVector;
		size_t rotatedVectorSize = sizeof(RotatedVector)*_vertexCount;
//		bool scratchFlag = false;
//		if(scratchPadMemory->BytesFree() >= rotatedVectorSize)
//		{
//			scratchFlag = true;
//			rotatedVector = (RotatedVector*)scratchPadMemory->Allocate(rotatedVectorSize);
//		}
//		else
//			rotatedVector = (RotatedVector*)HALLmalloc.Allocate(rotatedVectorSize);
		rotatedVector = new (HALScratchLmalloc) RotatedVector;

		assert(ValidPtr(rotatedVector ));
		scratchVariables->rv.rotatedVectorList = rotatedVector;
		register Vertex3D* vertexList = _vertexList;
		rotatedVector--;			    // allow increment during gte operation
		for(register int vertexIndex=_vertexCount;vertexIndex;vertexIndex--)
		{
			gte_ldv0(&vertexList->position);
			gte_rtps();
			rotatedVector++;
			rotatedVector->_originalVector = vertexList->position;
			vertexList++;
			gte_stsv(&rotatedVector->_resultingVector);
			gte_stsxy(&rotatedVector->_xy);
			gte_stsz(&rotatedVector->_z);
			gte_stflg(&rotatedVector->_flags);
			rotatedVector->_flags &= (~(
				GTE_FLAGF_LIME_OUTOFRANGE
					|GTE_FLAGF_LIMD1_OUTOFRANGE
					|GTE_FLAGF_LIMD2_OUTOFRANGE
				|GTE_FLAGF_SUM
				));

			gte_stir0(&rotatedVector->_ir0);
		}
		viewPort.Validate();

		scratchVariables->rv.vertexList = _vertexList;
		scratchVariables->rv.currentRenderFace = _faceList;
		Primitive* primitive  = _primList[viewPort.GetConstructionOrderTableIndex()];
		scratchVariables->rv.facesLeft = _faceCount;
//		int currentMaterial;
		while(scratchVariables->rv.facesLeft)
		{
			viewPort.Validate();
//			DBSTREAM3( cdebug << "Drawing face " << faceIndex << endl; )

			assert(scratchVariables->rv.currentRenderFace->materialIndex >= 0);
			assert(scratchVariables->rv.currentRenderFace->materialIndex < 100);  // kts arbitrary
			scratchVariables->rv.currentRenderMaterial = &_materialList[scratchVariables->rv.currentRenderFace->materialIndex];
			assert(ValidPtr(scratchVariables->rv.currentRenderMaterial));
			assert(ValidPtr(&scratchVariables->rv.currentRenderMaterial->_cdColor));
			pRenderObj3DFunc renderer = scratchVariables->rv.currentRenderMaterial->Get3DRenderer();

#if 0
			currentMaterial = scratchVariables->rv.currentRenderFace->materialIndex;
			while(scratchVariables->rv.currentRenderFace->materialIndex == currentMaterial)
			{
				(this->*renderer)(*primitive);
				scratchVariables->rv.currentRenderFace++;
				scratchVariables->rv.facesLeft--;
				primitive++;
			}
#else
			int offset = (this->*renderer)(primitive);
			scratchVariables->rv.facesLeft -= offset;
			primitive += offset;
#endif
			AssertMsg(scratchVariables->rv.facesLeft >= 0,"facesLeft = " << scratchVariables->rv.facesLeft << ", offset = " << offset);
		}

//		if(scratchFlag)
//			scratchPadMemory->Free(scratchVariables->rv.rotatedVectorList ,rotatedVectorSize);
//		else
			HALScratchLmalloc.Free(scratchVariables->rv.rotatedVectorList, rotatedVectorSize);
//	}
}

//============================================================================

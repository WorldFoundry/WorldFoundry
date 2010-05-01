//=============================================================================
// RenderObject2D.cc: 2D renderable object
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

#include <memory/memory.hp>
#include <gfx/rendobj2.hp>
#include <gfx/material.hp>
#if defined( __PSX__ )
#	include <libgte.h>
#	include <libgpu.h>
#endif

//============================================================================
// keeps pointers to facelist and vertexlist

RenderObject2D::RenderObject2D(Memory& memory, int vertexCount,Vertex2D* vertexList, int faceCount, TriFace* faceList, const Material* materialList)
{
	assert(vertexCount > 0);
	assert(faceCount > 0);
	assert(vertexCount >= faceCount);

	_vertexCount = vertexCount;
	_vertexList = vertexList;

	_faceCount = faceCount;
	_faceList = faceList;

#if DO_ASSERTIONS
	for(int idxFace = 0;idxFace < _faceCount;idxFace++)
	{
		assert(_faceList[idxFace].v1Index < _vertexCount);
		assert(_faceList[idxFace].v2Index < _vertexCount);
		assert(_faceList[idxFace].v3Index < _vertexCount);
	}
#endif
#if defined(USE_ORDER_TABLES)
	_primList[0] = new(memory) Primitive[ORDER_TABLES*_faceCount];
	assert(ValidPtr(_primList[0]));
	for ( int idxDisplay=1; idxDisplay<ORDER_TABLES; ++idxDisplay )
	{
		_primList[idxDisplay] = _primList[0] + (_faceCount*idxDisplay);			// kts to reduce # of allocations
	}

	for ( int idxPage=0; idxPage<ORDER_TABLES; ++idxPage )
		for ( int idxFace=0; idxFace<_faceCount; ++idxFace )
		{
			setPolyF3( &_primList[idxPage][idxFace].base );
		}
#endif
	ApplyMaterial( materialList );
}

//============================================================================

RenderObject2D::~RenderObject2D()
{
}

//============================================================================

void
RenderObject2D::ApplyMaterial(const Material* materialList)
{
#if defined(USE_ORDER_TABLES)
	for(int idxPage=0;idxPage<ORDER_TABLES;idxPage++)
		for(int idxFace=0;idxFace<_faceCount;idxFace++)
		{
			Material material = materialList[_faceList[idxFace].materialIndex];
			Color color = material.GetColor();
			setRGB0((P_TAG*)&_primList[idxPage][idxFace], color.Red(),color.Green(),color.Blue());
		}
#endif
}

//============================================================================

inline void
SetPoints(POLY_F3& prim,const Vector2& position,const Vector2& vertex1, const Vector2& vertex2, const Vector2& vertex3)
{
	prim.x0 = vertex1.X().WholePart() + position.X().WholePart();
	prim.y0 = vertex1.Y().WholePart() + position.Y().WholePart();
	prim.x1 = vertex2.X().WholePart() + position.X().WholePart();
  	prim.y1 = vertex2.Y().WholePart() + position.Y().WholePart();
	prim.x2 = vertex3.X().WholePart() + position.X().WholePart();
	prim.y2 = vertex3.Y().WholePart() + position.Y().WholePart();
}

//============================================================================

void
RenderObject2D::Render(ViewPort& vp,const Vector2& position, int depth)
{
	for ( int idxFace=0; idxFace < _faceCount; ++idxFace )
	{
#if defined(USE_ORDER_TABLES)
		SetPoints( *(POLY_F3*)&_primList[vp.GetConstructionOrderTableIndex()][idxFace], position,
			_vertexList[_faceList[idxFace].v1Index].position,
			_vertexList[_faceList[idxFace].v2Index].position,
			_vertexList[_faceList[idxFace].v3Index].position );
		vp.AddPrimitive(_primList[vp.GetConstructionOrderTableIndex()][idxFace],depth);
#else
	assert(0);
#endif
	}
}

//============================================================================

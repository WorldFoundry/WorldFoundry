//=============================================================================
// RenderObject3D.cc: 3D renderable object
// Copyright ( c ) 1997,1998,1999,2000,2001,2002 World Foundry Group  
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

#include <hal/hal.h>
#include <gfx/rendobj3.hp>
#include <math/vector3.hp>
#include <iff/iffread.hp>
#include "handle.hp"
#include "material.hp"
#include "callbacks.hp"

#if defined ( __PSX__ )
#	include <libgte.h>
#	include <libgpu.h>
#	include <inline_c.h>
#elif defined ( __WIN__ )
#	include <new.h>
#endif

#if defined(RENDERER_PIPELINE_SOFTWARE)
	#include <gfx/softwarepipeline/rendobj3.cc>
#elif defined ( RENDERER_PIPELINE_PSX )
	#include <gfx/psx/rendobj3.cc>
#elif defined ( RENDERER_PIPELINE_GL )
	#include <gfx/glpipeline/rendobj3.cc>
#elif defined ( RENDERER_PIPELINE_DIRECTX )
	#include <gfx/directxpipeline/rendobj3.cc>
#else
	#error no platform specific rendobj3 code!
#endif

//============================================================================
// keeps pointers to facelist and vertexlist

RenderObject3D::RenderObject3D( Memory& memory, int vertexCount,Vertex3D* vertexList, int faceCount, TriFace* faceList, const Material* materialList )
{
	_handleCount = 0;
	Construct(memory, vertexCount,vertexList,faceCount,faceList,materialList);
}

//=============================================================================

void
RenderObject3D::_InitPrimList( Memory& memory )
{
	_primList[0] = new (memory) Primitive[ORDER_TABLES*_faceCount];
	assert(ValidPtr(_primList[0]));
	for(int index=1;index<ORDER_TABLES;index++)
	{
		_primList[index] = _primList[0] + (_faceCount*index);			// kts to reduce # of allocations
	}
}

//=============================================================================

RenderObject3D::RenderObject3D( Memory& memory, const RenderObject3D& obj3d )
{
#pragma message( __FILE__ ": use reference counting" )

	_materialList = obj3d._materialList;

	_handleCount = obj3d._handleCount;
	_handleList = obj3d._handleList;
	_vertexCount = obj3d._vertexCount;
	_vertexList = obj3d._vertexList;
	_faceCount = obj3d._faceCount;
	_faceList = obj3d._faceList;

#if defined(USE_ORDER_TABLES)
	_InitPrimList( memory );
#endif
	ApplyMaterials(_materialList);
	Validate();				// make sure we built a good one
}

//=============================================================================

void
RenderObject3D::Construct( Memory& memory, int vertexCount,Vertex3D* vertexList, int faceCount, TriFace* faceList, const Material* materialList )
{
	assert(vertexCount > 0);
	assert(faceCount > 0);

	_vertexCount = vertexCount;
	_vertexList = vertexList;

	_faceCount = faceCount;
	_faceList = faceList;

#if DO_ASSERTIONS
	for(int debugIndex = 0;debugIndex < _faceCount;debugIndex++)
	{
		assert(_faceList[debugIndex].v1Index < _vertexCount);
		assert(_faceList[debugIndex].v2Index < _vertexCount);
		assert(_faceList[debugIndex].v3Index < _vertexCount);
#if defined(__WIN__)
		AssertMsg((faceList[debugIndex].normal.Length()) > Scalar::zero,"normal = " << faceList[debugIndex].normal << ", length = " << faceList[debugIndex].normal.Length());
#endif
	}
#endif
	_InitPrimList( memory );
	ApplyMaterials(materialList);
	Validate();				// make sure we built a good one
}

//============================================================================

RenderObject3D::~RenderObject3D()
{
}

//============================================================================

void
RenderObject3D::ApplyMaterials(const Material* materialList )
{
	_materialList = materialList;
	for(int page=0;page<ORDER_TABLES;page++)
		for(int index=0;index<_faceCount;index++)
		{
			const TriFace& face = _faceList[index];
			const Material* pMaterial = &( materialList[face.materialIndex] );
			assert( pMaterial );
			assert(ValidPtr(&_primList[page][index]));
			pMaterial->InitPrimitive(_primList[page][index],
				_vertexList[face.v1Index],
				_vertexList[face.v2Index],
				_vertexList[face.v3Index] );
		}

#if defined(RENDERER_PIPELINE_DIRECTX)
	for(int index=0;index<_faceCount;index++)
	{
		TriFace& face = _faceList[index];
		const Material* pMaterial = &( materialList[face.materialIndex] );
		assert( pMaterial );
		pMaterial->InitPrimitive(face,
			_vertexList[face.v1Index],
			_vertexList[face.v2Index],
			_vertexList[face.v3Index] );
	}
#endif
}


//============================================================================
// construct to load from disk

RenderObject3D::RenderObject3D(Memory& memory, binistream& input,int32 userData, const GfxCallbacks& callbacks )
{
    IFFChunkIter meshIter(input);
	assert(meshIter.GetChunkID().ID() == IFFTAG('M','O','D','L'));
	// create RenderObject3D from model on disk data
	int vertexCount = 0;
	Vertex3D* vertexList = NULL;
	Material* materials = NULL;
	int materialCount = 0;
	TriFace* faceList = NULL;
	int faceCount = 0;
	_handleCount = 0;
	_handleList = NULL;

//#pragma message ("KTS: memory leak, these news are never freed")
// now tracked with LMalloc
	while(meshIter.BytesLeft() > 0)
	{
		IFFChunkIter* chunkIter = meshIter.GetChunkIter(HALScratchLmalloc);
//		ciffread << "chunkid = " << chunkIter->GetChunkID() << std::endl;
		switch(chunkIter->GetChunkID().ID())
		{
			case IFFTAG('V','R','T','X'):
			{
				assert(vertexList == NULL);  // if this fires, there is more than one VRTX chunk
//				ciffread << "VRTX: ";
				vertexCount = chunkIter->Size()/sizeof(Vertex3DOnDisk );
//				ciffread << "Count = " << vertexCount << std::endl;
				assert(chunkIter->Size()%sizeof(Vertex3DOnDisk) == 0);
				assert(vertexCount);
				vertexList = new (memory) Vertex3D[vertexCount];
				assert(ValidPtr(vertexList));
				Vertex3DOnDisk tempVertex;
				for(int count=0;count<vertexCount;count++)
				{
					chunkIter->ReadBytes(&tempVertex,sizeof(Vertex3DOnDisk));

					vertexList[count].u = Scalar::FromFixed32(tempVertex.u);
					vertexList[count].v = Scalar::FromFixed32(tempVertex.v);
					vertexList[count].color = tempVertex.color;
					vertexList[count].position = Vector3_PS(Scalar::FromFixed32(tempVertex.x),Scalar::FromFixed32(tempVertex.y),Scalar::FromFixed32(tempVertex.z));
				}
				assert(chunkIter->BytesLeft() == 0);
				break;
			}
#pragma message ("KTS: figure out how to share common resources between multiple models (or at least more than instance of the same model)")
			case IFFTAG('M','A','T','L'):
			{
				assert(materials == NULL);  // if this fires, there is more than one MATL chunk
				materialCount = chunkIter->Size()/sizeof(_MaterialOnDisk);
				assert(chunkIter->Size()%sizeof(_MaterialOnDisk) == 0);
//				ciffread << "materialCount = " << materialCount << std::endl;
				materials = new (memory) Material[materialCount];
				assert(ValidPtr(materials));
				for(int index=0;index < materialCount; index++)
				{
#pragma message ("KTS: this causes two memory copies, is there a better way?")
					_MaterialOnDisk mod;
					chunkIter->ReadBytes(&mod,sizeof(_MaterialOnDisk));
//					ciffread << "Read material, bytes left = " << chunkIter->BytesLeft() << std::endl;

					const Texture* texture = &emptyTexture;
                    const PixelMap* texturePixelMap = NULL;
					if(mod._materialFlags & Material::TEXTURE_MAPPED)
					{
						AssertMsg(strlen(mod.textureName),"invalid texture name in material");
						LookupTextureStruct lts = callbacks.LookupTexture(mod.textureName,userData);
                        texture = &lts.texture;
                        texturePixelMap = &lts.texturePixelMap;
					}
					new (&materials[index]) Material(mod,*texture,texturePixelMap);
					materials[index].Validate();
				}
				assert(chunkIter->BytesLeft() == 0);
				break;
			}
			case IFFTAG('F','A','C','E'):
			{
				assert(ValidPtr(vertexList));  			// need vertex list so we can calc normals
				assert(faceList == NULL);  // if this fires, there is more than one FACE chunk
				assert(ValidPtr(materials));  // for debugging purposes, we want to know how many materials there are (so we can validate the material indecies)

				faceCount = chunkIter->Size()/sizeof(_TriFaceOnDisk);
				assert(faceCount);
				assert(chunkIter->Size() % sizeof(_TriFaceOnDisk) == 0);
				faceList = new(memory) TriFace[faceCount+1];  // plus one dummy face
				assert(ValidPtr(faceList));
				TriFace* tempFaceList = new(memory) TriFace[faceCount];
				assert(ValidPtr(tempFaceList));
				for(int index=0;index < faceCount; index++)
				{
#pragma message ("KTS: this causes two memory copies, is there a better way?")
					// kts since TriFaceOnDisk is a sub-set of triface, this works
					chunkIter->ReadBytes(&tempFaceList[index],sizeof(_TriFaceOnDisk));
					Vector3 v0( PSToVector3(vertexList[tempFaceList[index].v1Index].position));
					Vector3 v1( PSToVector3(vertexList[tempFaceList[index].v2Index].position));
					Vector3 v2( PSToVector3(vertexList[tempFaceList[index].v3Index].position));
					tempFaceList[index].normal = Vector3ToPS12(CalculateNormal(v0,v1,v2));
#if defined(__WIN__)
					assert((tempFaceList[index].normal.Length()) > Scalar::zero);
#endif
					AssertMsg(tempFaceList[index].materialIndex < materialCount,"Object ??? tried to refernce material index " << tempFaceList[index].materialIndex  << " when there are only " << materialCount << " materials");
					RangeCheck(0,tempFaceList[index].materialIndex,materialCount);
				}
				assert(chunkIter->BytesLeft() == 0);
				const int MATERIAL_CLEARED = -1;
				int materialIndex = MATERIAL_CLEARED-1;

				// sort faces by material index
				int nextFreeFace=0;
				while(materialIndex != MATERIAL_CLEARED)
				{
					// find next unused material index
					materialIndex = MATERIAL_CLEARED;
					for(int faceIndex=0;faceIndex < faceCount&&materialIndex == MATERIAL_CLEARED;faceIndex++)
						materialIndex = tempFaceList[faceIndex].materialIndex;

					if(materialIndex != MATERIAL_CLEARED)
					{
						// now copy all of the faces which use this material to the facelist
						for(int index=0;index< faceCount;index++)
						{
							if(tempFaceList[index].materialIndex == materialIndex)
							{
								faceList[nextFreeFace] = tempFaceList[index];
								tempFaceList[index].materialIndex = MATERIAL_CLEARED;
								nextFreeFace++;
							}
						}
					}
				}
				AssertMsg(nextFreeFace == faceCount,"nextFreeFace = " << nextFreeFace << ", faceCount+1 = " << faceCount+1);
				faceList[faceCount].materialIndex = -1;  // kts 3/27/98 9:45AM
				memory.Free(tempFaceList,sizeof(TriFace)*faceCount);
				break;
			}

			case IFFTAG('H','N','D','L'):
			{
				assert(_handleList == NULL);  // if this fires, there is more than one HNDL chunk
				_handleCount = chunkIter->Size() / sizeof(Handle);
				assert(chunkIter->Size()%sizeof(Handle) == 0);
				assert(_handleCount);
				_handleList = new(memory) Handle[_handleCount];
				assert(ValidPtr(_handleList));
				chunkIter->ReadBytes(_handleList,chunkIter->Size());
				assert(chunkIter->BytesLeft() == 0);
				break;
			}

			default:
//				ciffread << "ignoring chunk with ID of " << chunkIter->GetChunkID() << std::endl;
				break;
		}
		MEMORY_DELETE(HALScratchLmalloc,chunkIter,IFFChunkIter);
		//delete chunkIter;
//		meshIter.NextChunk();
//		ciffread << "meshIter bytes left = " << meshIter.BytesLeft() << std::endl;
	}
	assert( meshIter.BytesLeft() == 0 );
	assert(vertexCount > 0);
	assert(faceCount > 0);
	Construct(memory,vertexCount,vertexList,faceCount,faceList, materials);
}

//============================================================================

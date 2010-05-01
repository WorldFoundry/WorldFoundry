//============================================================================
// rendcrow.cc: RenderActorScarecrow
// Copyright (c) 1997,1999,2000,2001,2002,2003 World Foundry Group.  
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

//============================================================================
// Original Author: William B. Norris IV
//============================================================================

#include <gfx/material.hp>
#include <gfx/rendobj3.hp>
#include <gfx/callbacks.hp>
#include <renderassets/rendacto.hp>

//============================================================================

static Texture crowEmptyTexture = {"",0,320,0,64,64,0,0,0,16};

static Vertex3D scarecrowVertexList[] =
{
#define SIZE 1
		Vertex3D( SCALAR_CONSTANT(0.0), SCALAR_CONSTANT(0.5),Color(128,128,128), Vector3_PS( PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(SIZE)) ),
		Vertex3D( SCALAR_CONSTANT(0.5), SCALAR_CONSTANT(0.5),Color(200,200,50), Vector3_PS( PS_SCALAR_CONSTANT(SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(SIZE)) ),
		Vertex3D( SCALAR_CONSTANT(0.0), SCALAR_CONSTANT(0.5),Color(128,0,0), Vector3_PS( PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE)) ),
		Vertex3D( SCALAR_CONSTANT(0.5), SCALAR_CONSTANT(0.5),Color(0,128,0), Vector3_PS( PS_SCALAR_CONSTANT(SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE)) ),
};

static Material scarecrowMaterialList[] =
{
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR,crowEmptyTexture,NULL),			// blue
	Material(Color( 40,168, 40),Material::FLAT_SHADED|Material::SOLID_COLOR,crowEmptyTexture,NULL),			// green
};

static TriFace scarecrowFaceList[] =
{
	TriFace( 0, 1, 2, 0, scarecrowVertexList ),
	TriFace( 1, 3, 2, 0, scarecrowVertexList )
};

Vertex3D*
MakeScarecrowVertices(Memory& memory, const Vector3& min, const Vector3& max)
{
	Vertex3D* vertexList = new ( memory ) Vertex3D[ 4 ];
	assert( ValidPtr( vertexList ) );

	memcpy( vertexList, scarecrowVertexList, sizeof( Vertex3D ) * 4 );

	vertexList[0].position = Vector3ToPS(Vector3( min.X(), min.Y(), max.Z() ));
	vertexList[1].position = Vector3ToPS(Vector3( max.X(), min.Y(), max.Z() ));
	vertexList[2].position = Vector3ToPS(Vector3( min.X(), min.Y(), min.Z() ));
	vertexList[3].position = Vector3ToPS(Vector3( max.X(), min.Y(), min.Z() ));

	return vertexList;
}


//static void FillInScarecrowMaterialList( const Texture* texture, Material* materialList, Scalar weight = Scalar::half );

static void
FillInScarecrowMaterialList( const Texture* texture, const PixelMap* texturePixelMap, Material* materialList, Scalar weight )
{
	assert( ValidPtr( materialList ) );
	for ( int index=0; index < 2; ++index )
	{
		Color color( Color::white );
		materialList[ index ] = Material( color * weight, Material::FLAT_SHADED|Material::SOLID_COLOR, *texture, texturePixelMap);
	}
}


static Material*
MakeScarecrowMaterialList( Memory& memory, const Texture* texture, const PixelMap* texturePixelMap, Scalar weight )
{
	Material* materialList = new ( memory ) Material[2];
	assert( ValidPtr( materialList ) );
	FillInScarecrowMaterialList( texture, texturePixelMap, materialList, Scalar::one - weight );
	return materialList;
}


RenderActorScarecrow::RenderActorScarecrow( Memory& memory ) :
	RenderActor3D( memory, 4, MakeScarecrowVertices( memory, Vector3::zero, Vector3::one ), 2, scarecrowFaceList, scarecrowMaterialList )
{
	assert( 0 );
}


//static const Texture* _texture[ 100 ];

Scalar
fnChannel_Floor( Scalar timeVal )
{
	return Scalar( timeVal.Floor(), 0 );
}


RenderActorScarecrow::RenderActorScarecrow(Memory& memory, binistream& input,int32 userData, const GfxCallbacks& callbacks, const Vector3& min, const Vector3& max ) :
	RenderActor3D( memory, 4, MakeScarecrowVertices( memory, min, max ),
		2, scarecrowFaceList, scarecrowMaterialList )
{
	IFFChunkIter meshIter( input );

	assert( meshIter.GetChunkID() == ChunkID( IFFTAG('C','R','O','W') ) );
	// create RenderObject3D from model on disk data
	while(meshIter.BytesLeft() > 0)
	{
		IFFChunkIter* chunkIter = meshIter.GetChunkIter(HALScratchLmalloc);
//		ciff << "chunkid = " << chunkIter->GetChunkID() << std::endl;
		switch(chunkIter->GetChunkID().ID())
		{
			case IFFTAG('F','L','P','B'):
			{
				//std::cout << "FLPB" << std::endl;

				_nFrames = chunkIter->Size() - sizeof( int32 );
				//std::cout << "_nFrames = " << _nFrames << std::endl;

				int32 i;
				chunkIter->ReadBytes( &i, sizeof( int32 ) );
				_frameRate = Scalar( Scalar::FromFixed32(i) ).Invert();
				//std::cout << "Framerate is " << _frameRate;

				_flipbook = new (memory) uint8[_nFrames];
				assert( _flipbook );
				chunkIter->ReadBytes( _flipbook, _nFrames );

				break;
			}

			case IFFTAG('B','M','P','L'):
			{
				char* bitmapList = new (memory) char[chunkIter->Size()];
				assert( ValidPtr( bitmapList ) );
				chunkIter->ReadBytes( bitmapList, chunkIter->Size() );

//				std::cout << "size = " << chunkIter->Size();
//				std::cout << " bitmapList = [" << bitmapList << ']' << std::endl;

#if defined( USE_ASSET_ID )
				_nTextures = chunkIter->Size() / sizeof( long );
				_texture = new( memory )( Texture*[ _nTextures ] );
				assert( ValidPtr( _texture ) );
#else
                

				_nTextures = 0;
				char* p = bitmapList;
				char* pEnd = p + chunkIter->Size();
				while ( p < pEnd )
				{
					p += strlen( p ) + 1;
					++_nTextures;
				}
				assert( p == pEnd );

				_texture = new( memory )( const Texture*[ _nTextures ] );
				assert( ValidPtr( _texture ) );

				p = bitmapList;
				for ( int idxTexture = 0; idxTexture < _nTextures; ++idxTexture )
				{
					assert( p < pEnd );
					//std::cout << '[' << p << ']' << std::endl;
                    LookupTextureStruct lts = callbacks.LookupTexture( p, userData );
					_texture[ idxTexture ] = &lts.texture;
					assert( ValidPtr( _texture[ idxTexture ] ) );
                    if(idxTexture == 0)
                    {
                        _texturePixelMap = &lts.texturePixelMap;
                    }
                    else
                    {
                        assert(_texturePixelMap == &lts.texturePixelMap);
                    }
                    
					p += strlen( p ) + 1;
				}
#endif

				_materialList = MakeScarecrowMaterialList( memory, _texture[ 0 ], _texturePixelMap, Scalar::zero );
				assert( ValidPtr( _materialList ) );
				_object.ApplyMaterials( _materialList );
				break;
			}

			default :
//				ciff << "ignoring chunk with ID of " << chunkIter->GetChunkID() << std::endl;
				break;
		}
		MEMORY_DELETE(HALScratchLmalloc,chunkIter,IFFChunkIter);
		//delete chunkIter;
	}
	assert( meshIter.BytesLeft() == 0 );

	_channel = FnChannel( fnChannel_Floor, SCALAR_CONSTANT( 1 ) );
}

//============================================================================

RenderActorScarecrow::~RenderActorScarecrow()
{
}

//============================================================================

#if DO_VALIDATION
void
RenderActorScarecrow::_Validate() const
{
	ValidatePtr(this);
	ValidatePtr(_vertexList);
	_vertexList->Validate();
	ValidatePtr(_materialList);
	_materialList->Validate();
	RangeCheck(0,_nTextures,100);			// kts arbitrary
	ValidatePtr(_texture);
#if DO_VALIDATION > 1
	for(int textureIndex=0;textureIndex<_nTextures;textureIndex++)
	{
		ValidatePtr(_texture[textureIndex]);
//?		_texture[textureIndex]->Validate();
	}
#endif
	ValidatePtr(_flipbook);
	RangeCheck(0,_nFrames,100);			// kts arbitrary
}
#endif

//============================================================================

void
RenderActorScarecrow::Render( RenderCamera& renderCamera, const PhysicalObject& physicalObject, const Clock& currentTime )
{
	// Update frame based on time
	int nFrame = Scalar::FromFixed32( _channel.Value( currentTime.Current() ) ).WholePart();	// % Scalar( _nTextures, 0 );
	nFrame = nFrame % _nFrames;
	assert( 0 <= nFrame && nFrame < _nFrames );
	assert( _flipbook );
	nFrame = _flipbook[ nFrame ];

	//std::cout << "  nFrame = " << nFrame;
	assert( _nTextures );
	FillInScarecrowMaterialList( _texture[ nFrame ], _texturePixelMap, _materialList, Scalar( nFrame, 0 ) / _nTextures );
	_object.ApplyMaterials( _materialList );

	// Rotate to face camera, then render
	extern Euler cameraEuler;
	Euler e = cameraEuler;

	e.SetA( Angle::zero );
	e.SetB( Angle::zero );
	e.SetC( Angle::one - e.GetC() );

	Matrix34 matrix( e, physicalObject.GetPhysicalAttributes().Position() );
	renderCamera.RenderObject( _object, matrix);
}


void
RenderActorScarecrow::SetAnimationCycle( int )
{
}


bool
RenderActorScarecrow::GetHandle( const HandleID, Vector3& ) const
{
	return false;
}

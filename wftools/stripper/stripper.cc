//==============================================================================
// stripper.cc
// Copyright (c) 1996-1999, World Foundry Group  
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
//==============================================================================

#include "stripper.hp"
#include <iff/iffread.hp>
#include <streams/binstrm.hp>
#include <gfx/rendobj3.hp>
#include <hal/hal.h>
#include <gfx/material.hp>
#include <gfx/camera.hp>
#include <iffwrite/id.hp>
#include <iffwrite/iffwrite.hp>
#include <memory/lmalloc.hp>

#include "animcomp.hp"

#define ZMINLIMIT 7

extern Color clrBackground;
extern int windowWidth, windowHeight;
extern char szOutFile[];

static Texture dummyTexture = { "", 0, 320, 0, 64, 64 };

//-----------------------------------------------------------------------------
// this funcion needs to be connected to the texture asset system
const Texture&
LookupTexture( const char* name, int32 userData )                 // in streaming, this should be an asset ID
{
	return dummyTexture;
}

////////////////////////////////////////////////////////////////////////////////

Stripper::Stripper( RenderObject3D& ro3d ) :
	_ro3d( ro3d )
{
	_convertToStrips();
}

Stripper::~Stripper()
{
}


void
Stripper::_convertToStrips()
{
#if 0
	cout << "_vertexCount = " << _ro3d._vertexCount << endl;
	cout << "_vertexList = " << _ro3d._vertexList << endl;
	cout << "_faceCount = " << _ro3d._faceCount << endl;
	cout << "_faceList = " << _ro3d._faceList << endl;

	cout << "_materialList = " << _ro3d._materialList << endl;
#endif

	// Sort the faces by material number
	for ( int idxFace=0; idxFace<_ro3d._faceCount; ++idxFace )
	{
		const TriFace& pFace = _ro3d._faceList[ idxFace ];
		_face[ pFace.materialIndex ].push_back( pFace );
	}
}


_IffWriter& operator << ( _IffWriter& iff, const Color& color )
{
	int r = color.Red();
	int g = color.Green();
	int b = color.Blue();
	iff.out_int32( RGB( r, g, b ) );

	return iff;
}


_IffWriter& operator << ( _IffWriter& iff, const Material& mat )
{
	iff.out_int32( mat._materialFlags );	iff.out_comment( "Flags" );
	iff << mat._color;						iff.out_comment( "Flat-shaded color" );
	iff.out_string( mat._texture.szTextureName, MATERIAL_NAME_LEN );
	iff.out_printf_comment( "Texture name" );

	return iff;
}


_IffWriter& operator << ( _IffWriter& iff, const Vertex3D& v3 )
{
	iff.out_fixed( v3.u );
	iff.out_fixed( v3.v );
	iff << v3.color;
	iff.out_fixed( v3.position.X() );
	iff.out_fixed( v3.position.Y() );
	iff.out_fixed( v3.position.Z() );

	return iff;
}

//-----------------------------------------------------------------------------

void
Stripper::save( _IffWriter& _iff )
{
#if 0		// KINDA new way
	_iff.enterChunk( ID( 'MODL' ) );

		_iff.enterChunk( ID( 'ORGN' ) );
		{
			_iff.out_fixed( 0.0 );
			_iff.out_fixed( 0.0 );
			_iff.out_fixed( 0.0 );
		}
		_iff.exitChunk( ID( 'ORGN' ) );

		_iff.enterChunk( ID( 'MATL' ) );
		{
			for ( int idxMaterial=0; idxMaterial<4; ++idxMaterial )
			{
				_iff.out_printf_comment( "{ Material #%d", idxMaterial );
				_iff << _ro3d._materialList[ idxMaterial ];
				_iff.out_printf_comment( "End of Material #%d }", idxMaterial );
			}
		}
		_iff.exitChunk( ID( 'MATL' ) );

		_iff.enterChunk( ID( 'STRP' ) );
		{
			// temp output
			for ( int idxFace=0; idxFace<_ro3d._faceCount; ++idxFace )
			{
				_iff.out_int16( -1 );
				_iff.out_int16( _ro3d._faceList[ idxFace ].materialIndex );
				_iff.out_comment( "Material Index" );

				//_iff.out_int16( _ro3d._faceList[ idxFace ].v1Index );
				_iff << _ro3d._vertexList[ _ro3d._faceList[ idxFace ].v1Index ];

				//_iff.out_int16( _ro3d._faceList[ idxFace ].v2Index );
				_iff << _ro3d._vertexList[ _ro3d._faceList[ idxFace ].v2Index ];

				_iff.out_int16( _ro3d._faceList[ idxFace ].v2Index );
				_iff.out_printf_comment( "Initial triangle points" );
				_iff.out_int16( 1 );
				_iff.out_comment( "Number of triangles (# points = # triangles + 2)" );

				//_iff.out_int16( _ro3d._faceList[ idxFace ].v3Index );
				_iff << _ro3d._vertexList[ _ro3d._faceList[ idxFace ].v3Index ];
				_iff.out_comment( "" );
			}
		}
		_iff.exitChunk( ID( 'STRP' ) );

	_iff.exitChunk( ID( 'MODL' ) );
#endif

	_iff.enterChunk( ID( 'MODL' ) );

		_iff.enterChunk( ID( 'ORGN' ) );
		{
			_iff.out_fixed( 0.0 );
			_iff.out_fixed( 0.0 );
			_iff.out_fixed( 0.0 );
		}
		_iff.exitChunk( ID( 'ORGN' ) );

		_iff.enterChunk( ID( 'MATL' ) );
		{
			for ( int idxMaterial=0; idxMaterial < _materialCount; ++idxMaterial )
			{
				_iff.out_printf_comment( "{ Material #%d", idxMaterial );
				_iff << _ro3d._materialList[ idxMaterial ];
				_iff.out_printf_comment( "End of Material #%d }", idxMaterial );
			}
		}
		_iff.exitChunk( ID( 'MATL' ) );

		_iff.enterChunk( ID( 'VRTX' ) );
		{
			for ( int idxVertex=0; idxVertex<_ro3d._vertexCount; ++idxVertex )
			{
				_iff << _ro3d._vertexList[ idxVertex ];
				_iff.out_printf_comment( "Vertex #%d", idxVertex );
			}
		}
		_iff.exitChunk( ID( 'VRTX' ) );

		_iff.enterChunk( ID( 'STRP' ) );
		{
			// temp output
			for ( int idxFace=0; idxFace<_ro3d._faceCount; ++idxFace )
			{
				_iff.out_int16( _ro3d._faceList[ idxFace ].materialIndex );
				_iff.out_comment( "Material Index" );
				_iff.out_int16( _ro3d._faceList[ idxFace ].v1Index );
				_iff.out_int16( _ro3d._faceList[ idxFace ].v2Index );
				_iff.out_printf_comment( "Initial triangle points" );
				_iff.out_int16( 1 );
				_iff.out_comment( "Number of triangles (# points = # triangles + 2)" );
				_iff.out_int16( _ro3d._faceList[ idxFace ].v3Index );
				_iff.out_comment( "" );
			}
		}
		_iff.exitChunk( ID( 'STRP' ) );

	_iff.exitChunk( ID( 'MODL' ) );
}

//============================================================================

void
stripper( const char* szInputFilename )
{
	binistream binis( szInputFilename );

	ofstream iffOutStr( szOutFile, ios::out | ios::binary );
	assert( iffOutStr.good() );
	IffWriter iffOut( iffOutStr, false );
	LMalloc memory( 100000 );
	RenderObject3D* _object = NULL;

	while (!binis.eof())
	{
		IFFChunkIter chunkIter( binis );
		switch (chunkIter.GetSwappedChunkID().ID())
		{
#pragma message("Stripper is doing NO PROCESSING OF THE MODL CHUNK WHATSOEVER.")
//			case 'LDOM':	// Model chunk
//			{
//				_object = NEW( RenderObject3D( memory, chunkIter, 0 ) );
//				assert( ValidPtr( _object ) );

//				Stripper _stripper( *_object );
//				_stripper.save( iffOut );
//				break;
//			}


			case 'MODL':
			{
				iffOut.enterChunk( 'MODL' );
				int chunkSize = chunkIter.BytesLeft();
				char* chunkBuffer = (char*) malloc( chunkSize );
				chunkIter.ReadBytes( chunkBuffer, chunkSize );
				assert( chunkIter.BytesLeft() == 0);
				iffOut.out_mem( chunkBuffer, chunkSize );
				free(chunkBuffer);
				break;
			}
			case 'ANIM':	// Uncompressed animation chunk
			{
				AnimationCompressor animComp( chunkIter );
				animComp.Save( iffOut );
				break;
			}

			default:	// not one of our chunks, just copy it
			{
				// swap the bytes of the chunkID so IffWriter can use it
//				long unswappedID = chunkIter.GetChunkID().ID();
				long swappedID = chunkIter.GetSwappedChunkID().ID();
//				char* src = (char*)&unswappedID;	// ugly, isn't it?
//				char* dest = (char*)&swappedID;
//				dest[0] = src[3];
//				dest[1] = src[2];
//				dest[2] = src[1];
//				dest[3] = src[0];
				iffOut.enterChunk( ID(swappedID) );
				int chunkSize = chunkIter.BytesLeft();
				char* chunkBuffer = (char*) malloc( chunkSize );
				chunkIter.ReadBytes( chunkBuffer, chunkSize );
				assert( chunkIter.BytesLeft() == 0);
				iffOut.out_mem( chunkBuffer, chunkSize );
				free(chunkBuffer);
//				iffOut.exitChunk( ID(chunkIter.GetChunkID().ID()) );
				iffOut.exitChunk();
				break;
			}
		}

		assert( chunkIter.BytesLeft() == 0 );
	}

	// Open a window and preview the model

	if (_object)			// make sure there actually was a MODL chunk
	{
		Display display( 2, windowWidth, windowHeight );
		display.SetBackgroundColor( clrBackground );
		ViewPort vp( display, 1000, Scalar(windowWidth,0), Scalar(windowHeight,0) );

		RenderCamera camera( vp );
		Vector3 cameraPosition( Scalar::zero, Scalar::zero, Scalar::zero );
		Euler cameraRotation( Angle::zero, Angle(Angle::Degree(SCALAR_CONSTANT(180))), Angle::zero );

		Matrix34 camRot( cameraRotation, Vector3::zero );
		Matrix34 camPos( cameraPosition );

		camera.SetFog( Color::black, SCALAR_CONSTANT(10), SCALAR_CONSTANT(30) );
		camera.SetAmbientColor( Color(128,128,128) );
		camera.SetDirectionalLight( 0, Vector3(SCALAR_CONSTANT(1),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)) );
		camera.SetDirectionalLight( 1, Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(1),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0)) );
		camera.SetDirectionalLight( 2, Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(2)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1)) );

		Vector3 position( Scalar::zero, Scalar::zero, SCALAR_CONSTANT( ZMINLIMIT ) );
	//	Vector3 position( Scalar::zero, Scalar::zero, SCALAR_CONSTANT( 150 ) );

		Euler rot3D3( Angle::Degree(SCALAR_CONSTANT(109)), Angle::Degree(SCALAR_CONSTANT(73)), Angle::zero );

		for ( long i=0; i<2000; ++i )
		{
			rot3D3 += Euler( Angle::Degree(SCALAR_CONSTANT(3)),Angle::Degree(SCALAR_CONSTANT(2)),Angle::Degree(SCALAR_CONSTANT(0)) );

			Matrix34 camMat;
			camera.SetPosition( camMat = camPos * camRot );

			vp.Clear();
			camera.RenderBegin();

			Matrix34 mat3( rot3D3, position );
			camera.RenderObject( *_object, mat3 );

			camera.RenderEnd();
			vp.Render();
			display.PageFlip();
		}

		DELETE_CLASS( _object );
	}
}

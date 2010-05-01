//============================================================================
// gfxtest.cc:
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
// Description: The Display class encapsulates data and behavior for a single
//	 hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#define DEFINE_MAIN
//#define GTELAB

#define TEST_2D 0
#define TEST_3D 1
#define TEST_TEXTURES 1
#define TEST_MODEL_LOAD 0
#define TEST_EMITTER 1
#define ANIMATE_MATERIAL 0
#define ANIMATE_VERTEX 0
#define ANIMATE_POSITION 0

#if TEST_MODEL_LOAD
#if !DO_DEBUG_FILE_SYSTEM
#error Cannot test model loading without a file system!
#endif
#endif
#define MODEL_COUNT 1

//============================================================================

#include <hal/hal.h>

#include <math/scalar.hp>
#include <math/vector2.hp>
#include <math/vector3.hp>
#include <math/angle.hp>
#include <math/euler.hp>
#include <math/matrix34.hp>
#include <iff/iffread.hp>
#include <gfx/otable.hp>
#include <gfx/display.hp>
#include <gfx/viewport.hp>
#include <gfx/material.hp>
#include <gfx/rendobj2.hp>
#include <gfx/rendobj3.hp>
#include <gfx/camera.hp>
#include <gfx/vertex.hp>
#include <gfx/texture.hp>
#include <gfx/pixelmap.hp>
#if TEST_EMITTER
#include <particle/emitter.hp>
#endif

#if defined ( __PSX__)
#	include <sys/types.h>
#	include <libetc.h>
#	include <libgte.h>
#	include <libgpu.h>

#	include <profile/sampprof.hp>
#else
#endif

//-----------------------------------------------------------------------------

#if defined(DEFINE_MAIN)
//#if DO_DEBUGGING_INFO
//void
//breakpoint()
//	{
//	}
//extern "C" { extern int bDebugger; }							// kts used to enable int3 in assert
//#endif
#endif

//=============================================================================

#if defined(DEFINE_MAIN)
bool bShowWindow = false;
#if SW_DBSTREAM >= 1
CREATEANDASSIGNOSTREAM(casset,cout);
#endif
#endif

// bounce range
#define XYLIMIT 6
#define ZMAXLIMIT 18
#define ZMINLIMIT 7

//==========================================================================

static Texture dummyTexture = {"",0,320,0,64,64};

class GfxTestCallbacks : public GfxCallbacks
{
public:
   GfxTestCallbacks();
   virtual ~GfxTestCallbacks();

   virtual LookupTextureStruct LookupTexture(const char* name, int32 userData) const;                 // in streaming, this should be an asset ID

protected:
   virtual void _Validate() const;
};


GfxTestCallbacks::GfxTestCallbacks()
{

}

GfxTestCallbacks::~GfxTestCallbacks()
{

}

void
GfxTestCallbacks::_Validate() const
{

}

}

//-----------------------------------------------------------------------------
// this funcion needs to be connected to the texture asset system

LookupTextureStruct
GfxTestCallbacks::LookupTexture(const char* name, int32 userData) const                 // in streaming, this should be an asset ID
{
   assert(0);
   assert(testTexture);
	return(LookupTextureStruct(dummyTexture,*testTexture));
}

//=========================================================================

void
SetDelta2D(Vector2& position, Vector2& delta)
{
	if(position.X() > SCALAR_CONSTANT(300))
		delta.SetX(-delta.X());
	if(position.X() < SCALAR_CONSTANT(0))
		delta.SetX(-delta.X());

	if(position.Y() > SCALAR_CONSTANT(240))
		delta.SetY( -delta.Y());
	if(position.Y() < SCALAR_CONSTANT(0))
		delta.SetY( -delta.Y());
	position += delta;
}

//=============================================================================

void
SetDelta3D(Vector3& position, Vector3& delta)
{
	if(position.X() > SCALAR_CONSTANT(XYLIMIT))
		delta.SetX(-delta.X());
	if(position.X() < SCALAR_CONSTANT(-XYLIMIT))
		delta.SetX(-delta.X());

	if(position.Y() > SCALAR_CONSTANT(XYLIMIT))
		delta.SetY( -delta.Y());
	if(position.Y() < SCALAR_CONSTANT(-XYLIMIT))
		delta.SetY( -delta.Y());
	position += delta;

	if(position.Z() > SCALAR_CONSTANT(ZMAXLIMIT))
		delta.SetZ( -delta.Z());
	if(position.Z() < SCALAR_CONSTANT(ZMINLIMIT))
		delta.SetZ( -delta.Z());
	position += delta;
}

//=============================================================================

void
AdjustCameraParameters(Vector3& position,Euler& rotation,joystickButtonsF buttons1 )
{
#define ADJUST_ANGLE 1
#define ADJUST_DELTA 0.05

	// change the rotation angles for the cube and the light source
	if (buttons1 & EJ_BUTTONF_E)
		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (buttons1 & EJ_BUTTONF_A)
		rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (buttons1 & EJ_BUTTONF_D)
		rotation.SetB(rotation.GetB() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (buttons1 & EJ_BUTTONF_B)
		rotation.SetB(rotation.GetB() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (buttons1 & EJ_BUTTONF_C)
		rotation.SetC(rotation.GetC() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (buttons1 & EJ_BUTTONF_F)
		rotation.SetC(rotation.GetC() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));

	if (buttons1 & EJ_BUTTONF_UP)
		position.SetY(position.Y() + SCALAR_CONSTANT(ADJUST_DELTA));
	if (buttons1 & EJ_BUTTONF_DOWN)
		position.SetY(position.Y() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (buttons1 & EJ_BUTTONF_LEFT)
		position.SetX(position.X() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (buttons1 & EJ_BUTTONF_RIGHT)
		position.SetX(position.X() + SCALAR_CONSTANT(ADJUST_DELTA));

	if (buttons1 & EJ_BUTTONF_G)
		position.SetZ(position.Z() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (buttons1 & EJ_BUTTONF_H)
		position.SetZ(position.Z() + SCALAR_CONSTANT(ADJUST_DELTA));
#if defined( __PSX__ )
//	DBSTREAM1(cscreen << "camera position:" << endl << position << endl;)
//	DBSTREAM1(cscreen << "camera rotation:" << endl << rotation << endl;)
#endif
}

//=============================================================================

void
delay()
{
}

//=============================================================================

#include <pigsys/genfh.hp>

void
ReadFile(char* name,char* buffer, int size)
{
	assert( buffer );
	assert( name );
	assert( *name );

	int fp = FHOPENRD( name );
	AssertMsg(fp != -1,"cannot open file " << name);
	int fileSize = 0;
	fileSize = FHSEEKEND( fp, 0 );
	assert( fileSize > 0 );
	AssertMsg( fileSize < size, "fileSize = " << fileSize << "  buffer size = " << size );
	FHSEEKABS( fp, 0 );
#if DO_ASSERTIONS
	int actual =
#endif
	FHREAD( fp, (char*)buffer, fileSize );
	assert( actual == fileSize );
	FHCLOSE( fp );
}

//=============================================================================

#if TEST_TEXTURES

const int BUFFER_SIZE = 100*101;
long unsigned int * buffer[BUFFER_SIZE];

#if !DO_DEBUG_FILE_SYSTEM
#include "testtga.c"
#endif

void
LoadTextures(PixelMap& vram)
{
	PixelMap subMap(vram,320,0,64,64);
#if !DO_DEBUG_FILE_SYSTEM
	binistream texturestream((const void*)test,testSize);
#else
	binistream texturestream( "test.tga" );
#endif
	LoadTexture(texturestream,subMap);
}
#endif

//=============================================================================

#if DO_TEST_CODE
extern void ViewVideoMemory();
#endif


static Material cubeMaterialList[] =
{
#if 1
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40,168, 40),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // green
	Material(Color( 40,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // cyan
//      Material(Color( 40,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // cyan
	Material(Color(168, 40, 40),Material::GOURAUD_SHADED|Material::TEXTURE_MAPPED|Material::LIGHTING_LIT,dummyTexture),                     // red
	Material(Color(128,128,128),Material::FLAT_SHADED|Material::TEXTURE_MAPPED|Material::LIGHTING_LIT,dummyTexture),                        // yellow
	Material(Color(168, 40,168),Material::GOURAUD_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                        // pink
	Material(Color(168,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // white
//      Material(Color(168,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // white
//      Material(Color(168,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture)                    // white


#else
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
	Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),                   // blue
#endif
};

//=============================================================================

class GFXTester
{
public:
	GFXTester();
	~GFXTester();
	void Update();
	void Render();
	void PageFlip();
	Display& GetDisplay();
private:
	LMalloc testMemory;
	Display display;
	PixelMap vram;
	ViewPort vp;

	IJoystick			joy1, joy2;
	joystickButtonsF	buttons1, buttons2;

	Clock _clock;

#if TEST_2D
	Vertex2D vertexList[4];
	Material materialList[2];
	TriFace faceList[3];

	RenderObject2D* object;
	RenderObject2D* object2;
	RenderObject2D* object3;
	Vector2 position;
	Vector2 delta;
	Vector2 position2;
	Vector2 delta2;
	Vector2 position3;
	Vector2 delta3;
	Scalar xDelta;
#endif
#if TEST_3D
	Vertex3D cubeVertexList[8];
	TriFace cubeFaceList[13];

	RenderObject3D* object3D;
	Vector3 position3D;
	Vector3 delta3D;
	Euler rot3D;
	Euler deltaRot3D;

	RenderObject3D* object3D2;
	Vector3 position3D2;
	Vector3 delta3D2;
	Euler rot3D2;
	Euler deltaRot3D2;

	RenderObject3D* object3D3;
	Vector3 position3D3;
	Vector3 delta3D3;
	Euler rot3D3;
	Euler deltaRot3D3;

	RenderCamera camera;
	Vector3 cameraPosition;
//      Vector3 cameraPosition( SCALAR_CONSTANT(-4), SCALAR_CONSTANT(-4), SCALAR_CONSTANT(24) );
	Euler cameraRotation;
//      Euler cameraRotation = Euler(Angle(Angle::Degree(SCALAR_CONSTANT(7.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(187.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(0))));
#endif
#if TEST_MODEL_LOAD
	RenderObject3D* objectArray[MODEL_COUNT];
#endif
#if TEST_EMITTER
	Emitter* pEmitter;
	Vector3 posEmitter;
	Vector3 deltaPosEmitter;
#endif
};

//=============================================================================

extern int _halWindowWidth, _halWindowHeight;

GFXTester::GFXTester() :
	testMemory(HALLmalloc,500000 MEMORY_NAMED( COMMA "GFX test Memory" )),
#if defined(DO_STEREOGRAM)
	display( 2, 0,0,320, 240, testMemory, true ),
#else
	display( 2, 0,0,320, 240, testMemory ),
#endif
	vram(PixelMap::MEMORY_VIDEO,Display::VRAMWidth, Display::VRAMHeight),
	vp(display,9999,Scalar( 320, 0 ), Scalar( 240, 0 ), testMemory, 2 ),
	position3D(SCALAR_CONSTANT(-5),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMAXLIMIT)),
	delta3D(SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.1333),SCALAR_CONSTANT(0.3)),
	rot3D(Angle::zero,Angle::zero, Angle::zero),
	deltaRot3D(Angle::Degree(SCALAR_CONSTANT(4)),Angle::Degree(SCALAR_CONSTANT(5)),Angle::Degree(SCALAR_CONSTANT(0))),
	position3D2(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMAXLIMIT)),
	delta3D2(SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.0666),SCALAR_CONSTANT(0.2666)),
	rot3D2(Angle::zero,Angle::zero, Angle::zero),
	deltaRot3D2(Angle::Degree(SCALAR_CONSTANT(3)),Angle::Degree(SCALAR_CONSTANT(0)),Angle::Degree(SCALAR_CONSTANT(0))),
	position3D3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMINLIMIT)),
	delta3D3(SCALAR_CONSTANT(0.1333),SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.2333)),
	rot3D3(Angle::zero,Angle::zero,Angle::zero),
	deltaRot3D3(Angle::Degree(SCALAR_CONSTANT(3)),Angle::Degree(SCALAR_CONSTANT(2)),Angle::zero),
#if TEST_EMITTER
	posEmitter( Vector3::zero ),
	deltaPosEmitter( Vector3::zero ),
#endif
	camera(vp)

{
	joy1 = JoystickNew(EJW_JOYSTICK1);
	joy2 = JoystickNew(EJW_JOYSTICK2);

	display.SetBackgroundColor(Color(60,60,60));
#if defined( __PSX__ )
	PadInit(0);
#endif


#if 0

1
250  0 0 -1   1  1 0 0 1  .4 .8 .01 100 0
0


        sscanf(buf, "%d %d %d %d %f %f %f %f %f %f %f %f %d %d",
                        &num_points, &center_x, &center_y, &center_z,
                        &initialRadius, &initial_red, &initial_green,
                        &initial_blue,  &initialAlpha, &timestep,
                        &initial_velocity, &alpha_decrement, &period, &startDelay);
250  0 0 -1   1
 1 0 0
 1
 .4  timestep
 .8 vel
 .01 alpha dec
 100 period
 0 startDelay
#endif

#if TEST_2D
	vertexList[0] = Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(-40),SCALAR_CONSTANT( 50)) );
	vertexList[1] =	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT( 40),SCALAR_CONSTANT( 50)) );
	vertexList[2] =	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT( 40),SCALAR_CONSTANT(-50)) );
	vertexList[3] =	Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(-40),SCALAR_CONSTANT(-50)) );

	materialList[0] = Material(Color(168,20,20),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture);
	materialList[1] = Material(Color(20,20,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture);

	faceList[0] = TriFace(0,1,2,0);
	faceList[1] = TriFace(0,2,3,1);
	faceList[2] = TriFace(0,0,0,-1);	// end of list marker

	object = new (testMemory) RenderObject2D(testMemory,4,vertexList,2,faceList,materialList);
	assert(ValidPtr(object));
	object2 = new (testMemory) RenderObject2D(testMemory,4,vertexList,2,faceList,materialList);
	assert(ValidPtr(object2));
	object3 = new (testMemory) RenderObject2D(testMemory,4,vertexList,2,faceList,materialList);
	assert(ValidPtr(object3));
	position = Vector2(SCALAR_CONSTANT(10),SCALAR_CONSTANT(10));
	delta = Vector2(SCALAR_CONSTANT(5),SCALAR_CONSTANT(5));
	position2 = Vector2(SCALAR_CONSTANT(10),SCALAR_CONSTANT(10));
	delta2 = Vector2(SCALAR_CONSTANT(2),SCALAR_CONSTANT(2));
	position3 = Vector2(SCALAR_CONSTANT(10),SCALAR_CONSTANT(10));
	delta3 = Vector2(SCALAR_CONSTANT(12),SCALAR_CONSTANT(12));
	xDelta = SCALAR_CONSTANT(1.5);
#endif

#if TEST_3D
	#define SIZE 0.05
	// cube
	cubeVertexList[0] = Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 1),Color(128,128,128), Vector3_PS( PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT( SIZE)) );
	cubeVertexList[1] = Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 1),Color(200,200,50), Vector3_PS( PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT( SIZE)) );
	cubeVertexList[2] = Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 1),Color(128,0,0), Vector3_PS( PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE)) );
	cubeVertexList[3] = Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 1),Color(0,128,0), Vector3_PS( PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT(-SIZE)) );
	cubeVertexList[4] = Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 0),Color(0,128,0), Vector3_PS( PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT( SIZE)) );
	cubeVertexList[5] = Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 0),Color(0,0,0), Vector3_PS( PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT( SIZE)) );
	cubeVertexList[6] = Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 0),Color(0,0,128), Vector3_PS( PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT(-SIZE)) );
	cubeVertexList[7] = Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 0),Color(128,128,0), Vector3_PS( PS_SCALAR_CONSTANT(-SIZE),PS_SCALAR_CONSTANT( SIZE),PS_SCALAR_CONSTANT(-SIZE)) );

	cubeFaceList[0] = TriFace(0,1,2,0,cubeVertexList);                       // front(top)
	cubeFaceList[1] = TriFace(1,3,2,0,cubeVertexList);
	cubeFaceList[2] = TriFace(4,0,2,1,cubeVertexList);                       // left side
	cubeFaceList[3] = TriFace(2,7,4,1,cubeVertexList);
	cubeFaceList[4] = TriFace(1,5,6,2,cubeVertexList);                       // right side
	cubeFaceList[5] = TriFace(3,1,6,2,cubeVertexList);
	cubeFaceList[6] = TriFace(4,5,1,3,cubeVertexList);                       // top(back)
	cubeFaceList[7] = TriFace(4,1,0,3,cubeVertexList);
	cubeFaceList[8] = TriFace(2,3,6,4,cubeVertexList);                       // bottom(front)
	cubeFaceList[9] = TriFace(6,7,2,4,cubeVertexList);
	cubeFaceList[10] = TriFace(5,4,7,5,cubeVertexList);                       // back(bottom)
	cubeFaceList[11] = TriFace(5,7,6,5,cubeVertexList);
	cubeFaceList[12] = TriFace(0,0,0,-1);  // end of face list marker

	object3D = new (testMemory) RenderObject3D (testMemory,8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
	assert(ValidPtr(object3D));

	object3D2 = new (testMemory) RenderObject3D (testMemory,8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
	assert(ValidPtr(object3D2));

	object3D3 = new (testMemory) RenderObject3D (testMemory,8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
	assert(ValidPtr(object3D3));

	cameraPosition = Vector3( Scalar::zero, Scalar::zero, Scalar::zero );
//      Vector3 cameraPosition( SCALAR_CONSTANT(-4), SCALAR_CONSTANT(-4), SCALAR_CONSTANT(24) );
	cameraRotation = Euler(Angle::zero,Angle(Angle::Degree(SCALAR_CONSTANT(180))),Angle::zero);
//      Euler cameraRotation = Euler(Angle(Angle::Degree(SCALAR_CONSTANT(7.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(187.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(0))));
#endif

#if TEST_TEXTURES
	LoadTextures(vram);
#endif
// kts load a model from disk
#if TEST_MODEL_LOAD
	for(int index=0;index<MODEL_COUNT;index++)
	{
		//binistream binis( "torus.iff" );
		binistream binis( "axis.iff" );

		IFFChunkIter meshIter(binis);
		DBSTREAM1( cout << "mesh chunkid = " << meshIter.GetChunkID() << endl; )
		assert(meshIter.GetChunkID() == ChunkID('LDOM'));

		objectArray[index] = new (testMemory) RenderObject3D(testMemory,meshIter,0);
		assert(ValidPtr(objectArray[index]));
	}
#endif

#if TEST_EMITTER
	Emitter::EmitterParameters ep;
	ep.nParticles = 1;
	ep.position = Vector3::unitNegativeZ;
	ep.initialSphereRadius = SCALAR_CONSTANT( 0.1 );
	ep.period = SCALAR_CONSTANT( 0.15 );
	ep.startDelay = Scalar::two;

	// particle parameters
	Emitter::ParticleParameters pp;
	pp.initialVelocity = SCALAR_CONSTANT( 0.3 );
	pp.initialRotation = Euler(
			Angle( Scalar::Random( SCALAR_CONSTANT( -0.2 ), SCALAR_CONSTANT( 0.2 ) ) ),
			Angle( Scalar::zero ),
			Angle( SCALAR_CONSTANT( 0.01 ) )
	);
	pp.templateObject = object3D;
	pp.initialAlpha = Scalar::one;
	pp.alphaDecrement = SCALAR_CONSTANT( 0.05 );

	pEmitter = new Emitter( testMemory, ep, pp, _clock );
	assert( ValidPtr( pEmitter ) );
#endif
}

//=============================================================================

GFXTester::~GFXTester()
{
#if TEST_2D
	if( object3 )
		MEMORY_DELETE( testMemory, object3,RenderObject2D);
	if( object2 )
		MEMORY_DELETE( testMemory, object2,RenderObject2D);
	if( object )
		MEMORY_DELETE( testMemory, object,RenderObject2D);
#endif
#if TEST_3D
	if( object3D3 )
		MEMORY_DELETE( testMemory, object3D3,RenderObject3D);
	if( object3D2 )
		MEMORY_DELETE( testMemory, object3D2,RenderObject3D);
	if( object3D )
		MEMORY_DELETE( testMemory, object3D,RenderObject3D);
#endif
	JoystickDelete(joy1);
	JoystickDelete(joy2);
}

//=============================================================================

Display&
GFXTester::GetDisplay()
{
	return display;
}

//=============================================================================

void
GFXTester::Update()
{
	buttons1 = JoystickGetButtonsF(joy1);
	buttons2 = JoystickGetButtonsF(joy2);

#if TEST_EMITTER
	assert( ValidPtr( pEmitter ) );
	pEmitter->Update( _clock );
#endif

#if TEST_2D
#if ANIMATE_MATERIAL
	// animate material
		Color temp = materialList[1].GetColor();
		temp.SetRed(temp.Red() + 10);
		if(temp.Red() > 240)
			temp.SetRed(0);
		materialList[1].SetColor(temp);
		object->ApplyMaterial(materialList);
#endif
#if ANIMATE_VERTEX
	// animate vertex
		Scalar x = vertexList[0].position.X();
		x += xDelta;
		if(x < SCALAR_CONSTANT(-70))
			xDelta = -xDelta;
		if(x > SCALAR_CONSTANT(-20))
			xDelta = -xDelta;
		vertexList[0].position.SetX(x);
#endif

	// animate positions
	SetDelta2D(position,delta);
	SetDelta2D(position2,delta2);
	SetDelta2D(position3,delta3);
#endif

#if TEST_3D
#if ANIMATE_POSITION
#if defined( __PSX__ )
//	if (((PadRead(1)) & PADselect) == 0)                    // freeze on select
#endif
	{
 		SetDelta3D(position3D,delta3D);
 		SetDelta3D(position3D2,delta3D2);
 		SetDelta3D(position3D3,delta3D3);
// 		rot3D += deltaRot3D;
// 		rot3D2 += deltaRot3D2;
// 		rot3D3 += deltaRot3D3;
	}
#endif
#endif
//	// update camera
	AdjustCameraParameters(cameraPosition,cameraRotation,buttons1);

	_clock.tick( _clock() + SCALAR_CONSTANT( 0.2 ) );
}

//============================================================================

void
GFXTester::Render()
{
	display.RenderBegin();
	Matrix34 camRot(cameraRotation,Vector3::zero);

#if defined(DO_STEREOGRAM)
	Vector3 camPos2(cameraPosition);
	Matrix34 camPos(camPos2);
#else
	Matrix34 camPos(cameraPosition);
#endif

	Matrix34 camMat = camPos * camRot;

	camera.SetPosition(camMat);

	camera.SetFog(Color(0,0,0),SCALAR_CONSTANT(10),SCALAR_CONSTANT(30));
	camera.SetAmbientColor(Color(128,128,128));
	camera.SetDirectionalLight(0,Vector3(SCALAR_CONSTANT(1),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(1,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(1),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(2,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(2)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1)));

		// actually draw them
		vp.Clear();

#if TEST_3D
		camera.RenderBegin();
		Matrix34 mat(rot3D,position3D);

//		camera.RenderObject(*object3D,mat);

		Matrix34 mat2(rot3D2,position3D2);
		camera.RenderObject(*object3D2,mat2);
#endif

#if TEST_EMITTER
	{
#if 0
		Vector3 position( posEmitter );
		//position += Vector3(SCALAR_CONSTANT(2)*Scalar(index%3,0),SCALAR_CONSTANT(2)*Scalar(index/3,0),Scalar::zero);
		Matrix34 mtxEmitter( Euler::zero, position );
		Matrix34 inverseMatrix;
		inverseMatrix[3] = Vector3::zero;
		inverseMatrix.InverseDetOne( mtxEmitter );
#endif

		assert( ValidPtr( pEmitter ) );
		pEmitter->Render( camera );
//		pEmitter->Render( camera, *object3D );
	}
#endif

#if TEST_MODEL_LOAD
		for(int index=0;index<MODEL_COUNT;index++)
		{
			Vector3 position(position3D3);
			position += Vector3(SCALAR_CONSTANT(2)*Scalar(index%3,0),SCALAR_CONSTANT(2)*Scalar(index/3,0),Scalar::zero);
			Matrix34 mat3(rot3D3,position);
			camera.RenderObject(*objectArray[index],mat3);
		}
#endif

#if TEST_2D
		object->Render(vp,position,50);
		object2->Render(vp,position2,51);
		object3->Render(vp,position3,52);
#endif
#if TEST_3D
		camera.RenderEnd();
#endif

		vp.Render();
//              FntPrint("\nFrame: %d\n",frameCount++);
//              cscreen << "\nEuler:\n" << rot3D3 << endl;
	display.RenderEnd();
}

//============================================================================

void
GFXTester::PageFlip()
{
	display.PageFlip();

#if defined( __PSX__ )
//              while (((PadRead(1)) & PADR1) != 0)                     // debounce triangle
//                      ;

#if DO_TEST_CODE
	if((PadRead(1) & (PADstart|PADselect)) == (PADstart|PADselect) )
	{
		DBSTREAM1( cout << "Viewing Video memory:" << endl; )
		ViewVideoMemory();
	}
#endif
//	while (((PadRead(1)) & PADstart) != 0)                  // freeze on select
//	{
//                      if(((PadRead(1)) & PADR1) != 0)                 // debounce down
//                              break;
//	}
//	if (((PadRead(1)) & PADselect) != 0)                    // freeze on select
//	{
//		for(int i=0;i<1000;i++)
//			for(int j=0;j<10;j++)
//				PadRead(1);
//	}
#endif

}

//==============================================================================

static GFXTester* tester;

void
TestGFXInit()
{
	printf("Test gfx init\n");
	tester = new (HALLmalloc) GFXTester;
	assert(ValidPtr(tester));
}

void
TestGFXUnInit()
{
	printf("Test gfx uninit\n");
	MEMORY_DELETE(HALLmalloc,tester,GFXTester);
}

void
TestGFXLoop()
{
	//printf("Test gfx loop\n");
	tester->Update();
	tester->Render();
}


void
TestGFXPageFlip()
{
	//printf("Test gfx page flip\n");
	tester->PageFlip();
}

//==============================================================================

void
TestGFX()
{
	printf("Test gfx\n");
	TestGFXInit();
	printf("Test gfx init done\n");
#if defined(__PSX__ ) && defined(DO_PROFILE)
	profileSample sampler;
		sampler.start();
#endif

#if defined(DO_SLOW_STEREOGRAM)
	InitVSyncCallback(tester->GetDisplay());
#endif
#if defined( __PSX__ )
	for (;;)
//	for ( long i=0; i<1000; ++i )
#else
//	for ( long i=0; i<1000; ++i )
	for (;;)
#endif
	{
#if defined(DO_SLOW_STEREOGRAM)
	if(((PadRead(1)) & PADselect) == 0)                  // freeze on select
	{
		tester->Update();

		Display& display(tester->GetDisplay());
		int localOffset = vsyncCallbackInterface.orderTableOffset ^ 2;
		int secondLocalOffset = localOffset+1;
		vsyncCallbackInterface.constructLock = localOffset;
		if(localOffset == vsyncCallbackInterface.renderLock)
		{
			secondLocalOffset = localOffset;
			localOffset = secondLocalOffset+1;
		}

		vsyncCallbackInterface.constructLock = localOffset;
		display.SetConstructionOrderTableIndex(localOffset);
		assert(localOffset != vsyncCallbackInterface.renderLock);
		if(localOffset == vsyncCallbackInterface.renderLock)
			printf(__FILE__ ":%d: renderLock falied\n",__LINE__);
		display.ClearConstructionOrderTable();
		tester->Render();
		assert(localOffset != vsyncCallbackInterface.renderLock);

		while(secondLocalOffset == vsyncCallbackInterface.renderLock)
			;				        // lock waiting for renderer to catch up

		vsyncCallbackInterface.constructLock = secondLocalOffset;
		assert(secondLocalOffset != vsyncCallbackInterface.renderLock);
		if(secondLocalOffset == vsyncCallbackInterface.renderLock)
			printf(__FILE__ ":%d: renderLock falied\n",__LINE__);
		display.SetConstructionOrderTableIndex(secondLocalOffset);
		display.ClearConstructionOrderTable();
		tester->Render();
		assert(secondLocalOffset != vsyncCallbackInterface.renderLock);

		vsyncCallbackInterface.constructLock = -1;
		vsyncCallbackInterface.orderTableOffset ^= 2;
		vsyncCallbackInterface.pagesDirty[0] = 1;
		vsyncCallbackInterface.pagesDirty[1] = 1;
	}

//		cscreen << "\n\nrate: " << Scalar::one / VSyncCallBackReadVSyncCount() << endl;

//		for(int j=0;j<10;j++)
//			for(int i=0;i<900000;i++)
//				delay();
#else
		TestGFXLoop();
		TestGFXPageFlip();
#endif
	// kts test code
#if 0
//	VSyncCallback( callback);
		TestGFXLoop();
		TestGFXPageFlip();
		TestGFXLoop();
		TestGFXPageFlip();
		TestGFXLoop();
		TestGFXPageFlip();

	while(1)			                // lock
	{
		tester->PageFlip();
	}
#endif

	}
	printf("shutting down\n");
#if defined(__PSX__ ) && defined(DO_PROFILE)
	sampler.stop();
	sampler.save( "test.smp" );
#endif
#if defined(DO_SLOW_STEREOGRAM)
	UnInitVSyncCallback();
#endif

	TestGFXUnInit();
}

//============================================================================
// command line parsing
// returns index of first argv which doesn't contain a '-' switch

#if defined(DEFINE_MAIN)

int
ParseCommandLine(int argc, char** argv)
{
	int index;
	const char szDebugger[] = "debugger";

	for( index=1; index < argc && argv[index] && ((*argv[index] == '-') || (*argv[index] == '/')); index++)
	{
		if ( 0 )
			;
//#if DO_DEBUGGING_INFO
//        if ( strncmp( argv[index]+1, (char*)szDebugger,strlen( szDebugger) ) == 0 )
//		{
//			bDebugger = true;
//		}
//#endif
#if SW_DBSTREAM > 0
		else if ( tolower( *( argv[index]+1 ) ) == 'p' )
			RedirectStandardStream(argv[index]+2);
#endif
		else
			DBSTREAM1( cerror << "game Error: Unrecognized command line switch \"" <<
				(argv[index]+1) << "\"" << endl );
	 }

	return(index);
}
#endif
//============================================================================

#if DO_DEBUG_FILE_SYSTEM
#ifdef __PSX__
#if defined(GTELAB)
extern void GTELab();
#endif
#endif
#endif

//============================================================================

#if defined(DEFINE_MAIN)

void
PIGSMain(int argc, char* argv[])
{
	printf("GFX Test Program ('%s', %d)\n", argv[0] ? argv[0] : "", argc);
	printf("argc = %d\n", argc);
	for(int index = 0;index < argc; ++index)
	{
		assert(argv[index]);
		printf("argv[%d] = %s\n",index, argv[index]);
	}
	//int commandIndex =
	ParseCommandLine(argc,argv);

#if DO_DEBUG_FILE_SYSTEM
#if defined( __PSX__)
#if defined(GTELAB)
//	GTELab();
#endif
#endif
#endif
	TestGFX();
	printf("gfx test complete\n");
	PIGSExit();
}
#endif

//============================================================================

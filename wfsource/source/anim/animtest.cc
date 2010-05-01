//============================================================================
// animtest.cc:
//============================================================================

#include <hal/hal.h>

#include <cpplib/scalar.hp>
#include <cpplib/vector2.hp>
#include <cpplib/vector3.hp>
#include <cpplib/angle.hp>
#include <cpplib/euler.hp>
#include <iff/iffread.hp>
#include <memory/memory.hp>
#include <gfx/otable.hp>
#include <gfx/display.hp>
#include <gfx/viewport.hp>
#include <gfx/material.hp>
#include <gfx/rendobj2.hp>
#include <gfx/rendobj3.hp>
#include <gfx/camera.hp>
#include <gfx/vertex.hp>
#include <anim/anim.hp>

#if defined ( __PSX__)
#	include <sys/types.h>
#	include <libetc.h>
#	include <libgte.h>
#	include <libgpu.h>
#else
#endif

//=============================================================================

bool bShowWindow = false;
#if SW_DBSTREAM
CREATEANDASSIGNOSTREAM(casset,cout); )
#endif

#define TEST_2D 0
#define ANIMATE_VERTEX 0
#define ANIMATE_MATERIAL 0

#define TEST_3D 1
#define TEST_MODEL_LOAD 1
#define ANIMATE_OBJECT 1

// bounce range
#define XYLIMIT 6
#define ZMAXLIMIT 18
#define ZMINLIMIT 7

//==========================================================================

static Texture dummyTexture = {"",0,320,0,64,64};

//-----------------------------------------------------------------------------
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
AdjustCameraParameters(Vector3& position,Euler& rotation)
{
#if defined( __PSX__ )
	u_long  padd = PadRead(1);

//      if (padd & PADselect)   ret = -1;

#define ADJUST_ANGLE 1
#define ADJUST_DELTA 0.25

	// change the rotation angles for the cube and the light source
	if (padd & PADRup)
		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (padd & PADRdown)
		rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (padd & PADRleft)
		rotation.SetB(rotation.GetB() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (padd & PADRright)
		rotation.SetB(rotation.GetB() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (padd & PADR1)
		rotation.SetC(rotation.GetC() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	if (padd & PADR2)
		rotation.SetC(rotation.GetC() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));

	if (padd & PADLup)
		position.SetY(position.Y() + SCALAR_CONSTANT(ADJUST_DELTA));
	if (padd & PADLdown)
		position.SetY(position.Y() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (padd & PADLleft)
		position.SetX(position.X() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (padd & PADLright)
		position.SetX(position.X() + SCALAR_CONSTANT(ADJUST_DELTA));

	if (padd & PADL1)
		position.SetZ(position.Z() - SCALAR_CONSTANT(ADJUST_DELTA));
	if (padd & PADL2)
		position.SetZ(position.Z() + SCALAR_CONSTANT(ADJUST_DELTA));
#endif

	// distance from screen
//      if (padd & PADR1)       vec.vz += 8;
//      if (padd & PADR2)       vec.vz -= 8;

#if defined( __PSX__ )
	DBSTREAM1(cscreen << "camera position:" << endl << position << endl;)
	DBSTREAM1(cscreen << "camera rotation:" << endl << rotation << endl;)
#endif
}

//=============================================================================

void
delay()
{
}

//=============================================================================

#if defined(__PSX__)

#include <libsn.h>

void
ReadFile(char* name,char* buffer, int size)
{
	int fp = PCopen(name, 0, 0);
	AssertMsg(fp != -1,"cannot open file " << name);
	int fileSize = 0;
	if(fp != -1)
	{
		fileSize = PClseek(fp, 0, 2);
		assert(fileSize > 0);
		AssertMsg(fileSize < size, "fileSize = " << fileSize << "  buffer size = " << size );
		PClseek(fp, 0, 0);
		int actual = PCread(fp, (char*)buffer, fileSize);
		assert(actual == fileSize);
		PCclose(fp);
	}
	else
		printf("File %s not found\n",name);
}

const int BUFFER_SIZE = 100*101;
long unsigned int * buffer[BUFFER_SIZE];

void
LoadTextures()
{
	// kts texture load test
    RECT rect1;
    rect1.x=320;
    rect1.y = 0;
    rect1.w=64;
    rect1.h=64;

	PCinit();
	// read raw image from disk
	ReadFile("test.raw",(char*)buffer,BUFFER_SIZE);

    // Load texture to VRAM
//    LoadImage(&rect1,rawData);
    LoadImage(&rect1,(long unsigned int *)buffer);
}
#endif

//=============================================================================

#if DO_TEST_CODE
extern void
ViewVideoMemory();
#endif


Material cubeMaterialList[] =
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



void
TestGFX()
{
#if TEST_2D
	// box
	Vertex2D vertexList[4] =
	{
		Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(-40),SCALAR_CONSTANT( 50)) ),
		Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT( 40),SCALAR_CONSTANT( 50)) ),
		Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT( 40),SCALAR_CONSTANT(-50)) ),
		Vertex2D( SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),Color(), Vector2( SCALAR_CONSTANT(-40),SCALAR_CONSTANT(-50)) )
	};

	Material materialList[2] =
	{
		Material(Color(168,20,20),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),
		Material(Color(20,20,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture),
	};

	TriFace faceList[2] =
	{
		TriFace(0,1,2,0),
		TriFace(0,2,3,1)
	};
#endif

#if TEST_3D
	#define SIZE 1
	// cube
	Vertex3D cubeVertexList[8] =
	{
		Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 1),Color(128,128,128), Vector3( SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT( SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 1),Color(200,200,50), Vector3( SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT( SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 1),Color(128,0,0), Vector3( SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT(-SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 1),Color(0,128,0), Vector3( SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT(-SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 0),Color(0,128,0), Vector3( SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT( SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 0),Color(0,0,0), Vector3( SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT( SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 1), SCALAR_CONSTANT( 0),Color(0,0,128), Vector3( SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT(-SIZE)) ),
		Vertex3D( SCALAR_CONSTANT( 0), SCALAR_CONSTANT( 0),Color(128,128,0), Vector3( SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT( SIZE),SCALAR_CONSTANT(-SIZE)) )
	};

	TriFace cubeFaceList[12] =
	{
		TriFace(0,1,2,0,cubeVertexList),                       // front(top)
		TriFace(1,3,2,0,cubeVertexList),
		TriFace(4,0,2,1,cubeVertexList),                       // left side
		TriFace(2,7,4,1,cubeVertexList),
		TriFace(1,5,6,2,cubeVertexList),                       // right side
		TriFace(3,1,6,2,cubeVertexList),
		TriFace(4,5,1,3,cubeVertexList),                       // top(back)
		TriFace(4,1,0,3,cubeVertexList),
		TriFace(2,3,6,4,cubeVertexList),                       // bottom(front)
		TriFace(6,7,2,4,cubeVertexList),
		TriFace(5,4,7,5,cubeVertexList),                       // back(bottom)
		TriFace(5,7,6,5,cubeVertexList)
	};
#endif

	Display display(2,320,240);
	display.SetBackgroundColor(Color(60,60,60));
	ViewPort vp(display,1000,320,240);
#if TEST_2D
	RenderObject2D object(4,vertexList,2,faceList,materialList);
	RenderObject2D object2(4,vertexList,2,faceList,materialList);
	RenderObject2D object3(4,vertexList,2,faceList,materialList);
	Vector2 position(SCALAR_CONSTANT(10),SCALAR_CONSTANT(10));
	Vector2 delta(SCALAR_CONSTANT(5),SCALAR_CONSTANT(5));
	Vector2 position2(SCALAR_CONSTANT(10),SCALAR_CONSTANT(10));
	Vector2 delta2(SCALAR_CONSTANT(2),SCALAR_CONSTANT(2));
	Vector2 position3(SCALAR_CONSTANT(10),SCALAR_CONSTANT(10));
	Vector2 delta3(SCALAR_CONSTANT(12),SCALAR_CONSTANT(12));
	Scalar xDelta = SCALAR_CONSTANT(1.5);
#endif

#if TEST_3D
	RenderObject3D object3D(HALLmalloc, 8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
	Vector3 position3D(SCALAR_CONSTANT(-5),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMAXLIMIT));
	Vector3 delta3D(SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.1333),SCALAR_CONSTANT(0.3));
	Euler rot3D(0,0,0);
	Euler deltaRot3D(Angle::Degree(SCALAR_CONSTANT(4)),Angle::Degree(SCALAR_CONSTANT(5)),Angle::Degree(SCALAR_CONSTANT(0)));

	RenderObject3D object3D2(HALLmalloc,8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
	Vector3 position3D2(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMAXLIMIT));
	Vector3 delta3D2(SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.0666),SCALAR_CONSTANT(0.2666));
	Euler rot3D2(0,0,0);
	Euler deltaRot3D2(Angle::Degree(SCALAR_CONSTANT(3)),Angle::Degree(SCALAR_CONSTANT(0)),Angle::Degree(SCALAR_CONSTANT(0)));

	RenderObject3D object3D3(HALLmalloc,8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
	Vector3 position3D3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMINLIMIT));
	Vector3 delta3D3(SCALAR_CONSTANT(0.1333),SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.2333));
	Euler rot3D3(Angle::Degree(SCALAR_CONSTANT(109)),Angle::Degree(SCALAR_CONSTANT(73)),0);
	Euler deltaRot3D3(Angle::Degree(SCALAR_CONSTANT(3)),Angle::Degree(SCALAR_CONSTANT(2)),Angle::Degree(SCALAR_CONSTANT(0)));

	RenderCamera camera(vp);
	Vector3 cameraPosition( Scalar::zero, Scalar::zero, Scalar::zero );
//      Vector3 cameraPosition( SCALAR_CONSTANT(-4), SCALAR_CONSTANT(-4), SCALAR_CONSTANT(24) );
	Euler cameraRotation(0,Angle(Angle::Degree(SCALAR_CONSTANT(180))),0);
//      Euler cameraRotation = Euler(Angle(Angle::Degree(SCALAR_CONSTANT(7.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(187.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(0))));
#endif

#if defined( __PSX__ )
	PadInit(0);
	LoadTextures();
#endif

// kts load a model from disk
#if TEST_MODEL_LOAD
#define MODEL_COUNT 1
	RenderObject3D* objectArray[MODEL_COUNT];
#if ANIMATE_OBJECT
	AnimateRenderObject3D* animArray[MODEL_COUNT];
#endif
	for(int index=0;index<MODEL_COUNT;index++)
	{
		binistream binis( "torus.iff" );

		{
			IFFChunkIter meshIter(binis);
			cout << "mesh chunkid = " << meshIter.GetChunkID() << endl;
			assert(meshIter.GetChunkID() == 'LDOM');

			objectArray[index] = new (HALLmalloc) RenderObject3D(HALLmalloc, meshIter,0);
			assert(ValidPtr(objectArray[index]));
		}                               // destruct chunkIter, stream is now at next chunk
#if ANIMATE_OBJECT
		IFFChunkIter animIter(binis);
		cout << "anim chunkid = " << animIter.GetChunkID() << endl;
		assert(animIter.GetChunkID() == 'MINA');
		animArray[index] = new (HALLmalloc) AnimateRenderObject3D(HALLmalloc,animIter,*(objectArray[index]));
#endif
	}
#endif

#if ANIMATE_OBJECT
	Scalar time = 0;
#endif

//      int frameCount = 0;
#if defined( __PSX__ )
	for (;;)
#else
//      for ( long i=0; i<10000000; ++i )
	for ( long i=0; i<400; ++i )
#endif
	{
#if TEST_2D
#if ANIMATE_MATERIAL
	// animate material
		Color temp = materialList[1].GetColor();
		temp.SetRed(temp.Red() + 10);
		if(temp.Red() > 240)
			temp.SetRed(0);
		materialList[1].SetColor(temp);
		object.ApplyMaterial(materialList);
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
 SetDelta3D(position3D,delta3D);
 SetDelta3D(position3D2,delta3D2);
 SetDelta3D(position3D3,delta3D3);
 rot3D += deltaRot3D;
 rot3D2 += deltaRot3D2;
 rot3D3 += deltaRot3D3;


#if ANIMATE_OBJECT
	for(int index=0;index<MODEL_COUNT;index++)
	{
		animArray[index]->Animate(time,*(objectArray[index]));
	}
	time += SCALAR_CONSTANT(0.2);

#endif

//	// update camera
	AdjustCameraParameters(cameraPosition,cameraRotation);

	Matrix34 camRot(cameraRotation,Vector3(0,0,0));
	Matrix34 camPos(cameraPosition);

	Matrix34 camMat = camPos * camRot;
	camera.SetPosition(camMat);

	camera.SetFog(Color(0,0,0),SCALAR_CONSTANT(10),SCALAR_CONSTANT(30));
	camera.SetAmbientColor(Color(128,128,128));
	camera.SetDirectionalLight(0,Vector3(SCALAR_CONSTANT(1),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(1,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(1),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(2,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(2)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1)));
#endif

		// actually draw them
		vp.Clear();
#if TEST_3D
		camera.RenderBegin();
		Matrix34 mat(rot3D,position3D);
		camera.RenderObject(object3D,mat);

		Matrix34 mat2(rot3D2,position3D2);
		camera.RenderObject(object3D2,mat2);
#endif

#if TEST_MODEL_LOAD
		for(index=0;index<MODEL_COUNT;index++)
		{
			Vector3 position(position3D3);
			position += Vector3(SCALAR_CONSTANT(2)*Scalar(index%3,0),SCALAR_CONSTANT(2)*Scalar(index/3,0),0);
			Matrix34 mat3(rot3D3,position);
			camera.RenderObject(*objectArray[index],mat3);
		}
#endif

#if TEST_2D
		object.Render(vp,position,50);
		object2.Render(vp,position2,51);
		object3.Render(vp,position3,52);
#endif
#if TEST_3D
		camera.RenderEnd();
#endif

		vp.Render();
//              FntPrint("\nFrame: %d\n",frameCount++);
//              cscreen << "\nEuler:\n" << rot3D3 << endl;

		display.PageFlip();

#if defined( __PSX__ )
//              while (((PadRead(1)) & PADR1) != 0)                     // debounce triangle
//                      ;

#if DO_TEST_CODE
		if((PadRead(1) & (PADstart|PADselect)) == (PADstart|PADselect) )
		{
			cout << "Viewing Video memory:" << endl;
			ViewVideoMemory();
		}
#endif
		while (((PadRead(1)) & PADstart) != 0)                  // freeze on select
		{
//                      if(((PadRead(1)) & PADR1) != 0)                 // debounce down
//                              break;
		}
		if (((PadRead(1)) & PADselect) != 0)                    // freeze on select
		{
			for(int i=0;i<1000;i++)
				for(int j=0;j<10;j++)
					PadRead(1);
		}
#endif
	}
}

//============================================================================
// command line parsing
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(int argc, char** argv)
{
	int index;
	const char szDebugger[] = "debugger";

	for( index=1; index < argc && argv[index] && ((*argv[index] == '-') || (*argv[index] == '/')); index++)
	{
        if ( strncmp( argv[index]+1, (char*)szDebugger,strlen( szDebugger) ) == 0 )
		{
			bDebugger = true;
		}
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

//============================================================================

extern void Cubes();

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

//      Cubes();

	TestGFX();
	printf("gfx test complete\n");
	PIGSExit();
}

//============================================================================

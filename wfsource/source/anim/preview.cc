//============================================================================
// preview.cc:	display geometry file and animations
//============================================================================

#include <hal/hal.h>

#include <math/scalar.hp>
#include <math/vector2.hp>
#include <math/vector3.hp>
#include <math/angle.hp>
#include <math/euler.hp>
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
#include <gfx/rmuv.hp>
#include <anim/anim.hp>
#include <anim/animcycl.hp>

#if defined ( __PSX__)
#	include <sys/types.h>
#	include <libetc.h>
#	include <libgte.h>
#	include <libgpu.h>
#else
#endif

//-----------------------------------------------------------------------------

#if DEBUG
void
breakpoint()
	{
	}
extern "C" { extern int bDebugger; }							// kts used to enable int3 in assert
#endif

//=============================================================================

#define ANIMATE_OBJECT 1

//=============================================================================

extern int _halWindowWidth, _halWindowHeight;

//bool bShowWindow = false;
//#if DEBUG
//ostream_withassign casset(cout);
//#endif

// bounce range
#define XYLIMIT 6
#define ZMAXLIMIT 18
#define ZMINLIMIT 7

//==========================================================================

#define TEXTURE_XPOS 320
#define TEXTURE_YPOS 0

static Texture dummyTexture = {"",0,TEXTURE_XPOS,TEXTURE_YPOS,64,64};

//-----------------------------------------------------------------------------
// this funcion needs to be connected to the texture asset system

RMUV* _ruv;

const Texture&
LookupTexture(const char* name, int32 userData)                 // in streaming, this should be an asset ID
{
	assert(ValidPtr(_ruv));
	_RMUV* rmuv = _ruv->GetRMUV(name);

	AssertMsg(ValidPtr(rmuv),"could not find texture <" << name << ">");
	return(*((Texture*)rmuv));
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
	int actual = FHREAD( fp, (char*)buffer, fileSize );
	assert( actual == fileSize );
	FHCLOSE( fp );
}

//=============================================================================

const int BUFFER_SIZE = 100*101;
long unsigned int * buffer[BUFFER_SIZE];

//=============================================================================

void
LoadRUV(binistream& ruvstream)
{
	int32 chunkName;
	ruvstream >> chunkName;
	assert(chunkName  == 'vumr');
	int32 size;
	ruvstream >> size;
	assert(size >= 0);
	assert(size < 10000);			    // some limit
//	char* _rmuvMemory = (char*)_assetsMemory.Allocate(sizeof(char)*(size+8));
	char* _rmuvMemory = new (HALLmalloc) char[size+8];
	assert(ValidPtr(_rmuvMemory));
	*((int32*)_rmuvMemory) = chunkName;
	*((int32*)(_rmuvMemory+4)) = size;
	ruvstream.read(_rmuvMemory+8,size);
	_ruv = new (HALLmalloc) RMUV(_rmuvMemory,TEXTURE_XPOS,TEXTURE_YPOS);
	DBSTREAM1( cout << (*_ruv) << endl; )
}

//=============================================================================

#if !DO_DEBUG_FILE_SYSTEM
#error preview does not work without a file system
#endif

void
LoadTextures(PixelMap& vram)
{
	PixelMap subMap(vram,TEXTURE_XPOS,TEXTURE_YPOS,256,512);
	subMap.Clear(Color(255,0,0));
	binistream texturestream( "perm.tga" );
	assert(texturestream.good());
	LoadTexture(texturestream,subMap);
	binistream ruvstream("perm.ruv");
	assert(ruvstream.good());
	LoadRUV(ruvstream);
}

//=============================================================================

#if DEBUG
extern void ViewVideoMemory();
#endif

void
DumpGTERegs();


void
Preview(char* objectFileName)
{
	IJoystick			joy1, joy2;
	joystickButtonsF	buttons1, buttons2;

	joy1 = JoystickNew(EJW_JOYSTICK1);
	joy2 = JoystickNew(EJW_JOYSTICK2);

//	while (!_JoystickUserAbort())
//		if ( b & EJ_BUTTONF_I )

	Scalar deltaTime(0);
	Scalar absoluteTime(0);
	int animationCycle = 0;
	Display display(2,320,240);

	PixelMap vram(PixelMap::MEMORY_VIDEO,Display::VRAMWidth, Display::VRAMHeight);
	display.SetBackgroundColor(Color(60,60,60));
	ViewPort vp(display,4000,Scalar( _halWindowWidth, 0 ), Scalar( _halWindowHeight, 0 ) );

	Vector3 position3D3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(ZMINLIMIT));
	Vector3 delta3D3(SCALAR_CONSTANT(0.1333),SCALAR_CONSTANT(0.1),SCALAR_CONSTANT(0.2333));
	Euler rot3D3(Angle::Degree(SCALAR_CONSTANT(109)),Angle::Degree(SCALAR_CONSTANT(73)),0);
	Euler deltaRot3D3(Angle::Degree(SCALAR_CONSTANT(3)),Angle::Degree(SCALAR_CONSTANT(2)),Angle::Degree(SCALAR_CONSTANT(0)));

	RenderCamera camera(vp);
	Vector3 cameraPosition( Scalar::zero, Scalar::zero, Scalar::zero );
//      Vector3 cameraPosition( SCALAR_CONSTANT(-4), SCALAR_CONSTANT(-4), SCALAR_CONSTANT(24) );
	Euler cameraRotation(0,Angle(Angle::Degree(SCALAR_CONSTANT(180))),0);
//      Euler cameraRotation = Euler(Angle(Angle::Degree(SCALAR_CONSTANT(7.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(187.998046))),Angle(Angle::Degree(SCALAR_CONSTANT(0))));

#if defined( __PSX__ )
	PadInit(0);
#endif
	LoadTextures(vram);

// kts load a model from disk
#define MODEL_COUNT 1
	RenderObject3D* objectArray[MODEL_COUNT];
#if ANIMATE_OBJECT
	AnimationCycleArray* animCycleArray[MODEL_COUNT];
//	AnimateRenderObject3D* animArray[MODEL_COUNT];
#endif
	for(int index=0;index<MODEL_COUNT;index++)
	{
		binistream binis( objectFileName );
		{
			IFFChunkIter meshIter(binis);
			DBSTREAM1( cout << "mesh chunkid = " << meshIter.GetChunkID() << endl; )
			assert(meshIter.GetChunkID().ID() == 'LDOM');

			objectArray[index] = new (HALLmalloc) RenderObject3D(HALLmalloc,meshIter,0);
			assert(ValidPtr(objectArray[index]));
			objectArray[index]->Validate();
		}
#if ANIMATE_OBJECT
		if(binis.good())
		{
		assert(binis.good());
		animCycleArray[index] = new (HALLmalloc) AnimationCycleArray(HALLmalloc, binis, *objectArray[index]);
		assert(ValidPtr(animCycleArray[index]));
		animCycleArray[index]->Validate();
		}
		else
			animCycleArray[index] = NULL;
//		IFFChunkIter animIter(binis);
//		DBSTREAM1( cout << "anim chunkid = " << animIter.GetChunkID() << endl; )
//		assert(animIter.GetChunkID() == 'MINA');
//		animArray[index] = NEW(AnimateRenderObject3D(HALLmalloc,animIter,*(objectArray[index])) );
#endif
	}

#if ANIMATE_OBJECT
	Scalar time = Scalar::zero;
#endif

	for (;;)
	{
#if defined(__PSX__)
    DBSTREAM1( cscreen << "Preview V0.2: cycle = " << animationCycle << endl; )
#endif
	buttons1 = JoystickGetButtonsF(joy1);
	buttons2 = JoystickGetButtonsF(joy2);

#if ANIMATE_OBJECT
	for(int index=0;index<MODEL_COUNT;index++)
	{
		if(animCycleArray[index])
		{
			animCycleArray[index]->SetCycle(animationCycle);
			animCycleArray[index]->Animate(time,*(objectArray[index]));
		}
	}
	time += deltaTime;
#endif

//	// update camera
	AdjustCameraParameters(cameraPosition,rot3D3,buttons1);

	Matrix34 camRot(cameraRotation,Vector3(Scalar::zero,Scalar::zero,Scalar::zero));
	Matrix34 camPos(cameraPosition);

	Matrix34 camMat = camPos * camRot;
	camera.SetPosition(camMat);

	camera.SetFog(Color(0,128,0),SCALAR_CONSTANT(50),SCALAR_CONSTANT(100));
	camera.SetAmbientColor(Color(128,128,128));
	camera.SetDirectionalLight(0,Vector3(SCALAR_CONSTANT(1),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(1,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(1),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(2,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(2)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1)));

		// actually draw them
		vp.Clear();
		camera.RenderBegin();

		for(int objectIndex=0;objectIndex<MODEL_COUNT;objectIndex++)
		{
			Vector3 position(position3D3);
			position += Vector3(SCALAR_CONSTANT(2)*Scalar(objectIndex%3,0),SCALAR_CONSTANT(2)*Scalar(objectIndex/3,0),Scalar::zero);
			Matrix34 mat3(rot3D3,position);
//			cscreen << rot3D3;
//			cscreen << mat3 << endl;
			Matrix34 inverseMatrix;
			inverseMatrix[3] = Vector3::zero;
			inverseMatrix.InverseDetOne(mat3);			// rotate lights into local coordinate space
			camera.RenderObject(*objectArray[objectIndex],mat3,inverseMatrix);
		}
//		DumpGTERegs();
		camera.RenderEnd();

		vp.Render();
		deltaTime = display.PageFlip();
#if defined(__PSX__)
		DBSTREAM1( cscreen << "deltaTime:" << deltaTime << endl; )
#endif
		if(deltaTime > SCALAR_CONSTANT(0.1))
			deltaTime = SCALAR_CONSTANT(0.1);
		absoluteTime += deltaTime;

#if defined( __PSX__ )
#if DEBUG
		if((PadRead(1) & (PADL2|PADL1|PADR2|PADR1)) == (PADL2|PADL1|PADR2|PADR1) )
		{
			DBSTREAM1( cout << "Viewing Video memory:" << endl; )
			ViewVideoMemory();
		}
#endif
#endif
//		while(JoystickGetButtonsF(joy1) & EJ_BUTTONF_I)
//			;		// freeze on start

		if(buttons1 & EJ_BUTTONF_I)
		{
			while (JoystickGetButtonsF(joy1) & EJ_BUTTONF_I)
				;
			animationCycle++;
			if(animationCycle > AnimationManager::MAX_ANIMATION_CYCLES)
				animationCycle = 0;
#if defined(__WIN__)
    DBSTREAM1( cout << "Preview V0.2: cycle = " << animationCycle << endl; )
#endif
		}
	}
	JoystickDelete(joy1);
	JoystickDelete(joy2);
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
		if ( 0 )
			;
#if DEBUG
        if ( strncmp( argv[index]+1, (char*)szDebugger,strlen( szDebugger) ) == 0 )
		{
			bDebugger = true;
		}
#endif
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

void
PIGSMain(int argc, char* argv[])
{
	printf("Preview V0.1 ('%s', %d)\n", argv[0] ? argv[0] : "", argc);
	printf("argc = %d\n", argc);
	for(int index = 0;index < argc; ++index)
	{
		assert(argv[index]);
		printf("argv[%d] = %s\n",index, argv[index]);
	}
	printf("Requires perm.ruv & perm.tga to contain all textures used by object\n");
	int commandIndex = ParseCommandLine(argc,argv);
	assert(commandIndex = argc-1);		// must enter a geometry file name

	Preview(argv[commandIndex]);
	printf("gfx test complete\n");
	PIGSExit();
}

//============================================================================

//=============================================================================
// gfxtest.cc:
// Copyright ( c ) 2002 World Foundry Group  
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
// Description: test physics syb-system & ode
// Original Author: Kevin T. Seghetti
//============================================================================
                         
/*************************************************************************
 * ODE interface Based on example from                                   *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 *************************************************************************/

#include <pigsys/pigsys.hp>
#include <baseobject/msgport.hp>
#include "physical.hp"
#include "ode/ode.hp"
#include "colbox.hp"
#include "colspace.hp"
#include <list>

#define TEST_TEXTURES 1

//============================================================================

#include <hal/hal.h>
#include <math/scalar.hp>
#include <math/vector3.hp>
#include <math/angle.hp>
#include <iff/iffread.hp>
#include <gfx/display.hp>
#include <gfx/viewport.hp>
#include <gfx/material.hp>
#include <gfx/rendobj3.hp>
#include <gfx/camera.hp>
#include <gfx/vertex.hp>
#include <gfx/texture.hp>
#include <gfx/pixelmap.hp>
#include <cpplib/stdstrm.hp>
#include <cpplib/strmnull.hp>

namespace ode
{
#include <ode/ode.h>
}

#ifdef MSVC
#pragma warning(disable:4244 4305)  // for VC++, no precision loss complaints
#endif

// some constants

//static dWorldID world;
static ode::dSpaceID space;
static ode::dJointGroupID contactgroup;
int random_pos = 1;
    
//=============================================================================

struct TestObject
{
   TestObject(const Vector3& position, const Euler& rotation, const Vector3& min, const Vector3& max) 
   {
      odeWorld.SetSpace(space);
      _physical.Construct(position,rotation,min,max);
   }

   RenderObject3D* object3D;
   PhysicalAttributes _physical;
};

//==============================================================================

typedef std::list<TestObject*>::iterator TestObjectIter;

#if 0
class TestObjectPhysicalIterator : public PhysicalAttributesIterator
{
public:
    TestObjectPhysicalIterator(const TestObjectIter& iter);
    PhysicalAttributes& operator*();
    void operator++();
    bool operator==(const PhysicalAttributesIterator&);
private:
    TestObjectIter _iter;
};

    
TestObjectPhysicalIterator::TestObjectPhysicalIterator(const TestObjectIter& iter)
{
    _iter = iter;
}

PhysicalAttributes&
TestObjectPhysicalIterator::operator*()
{
    return (*_iter)->_physical;
}

bool
TestObjectPhysicalIterator::operator==(const PhysicalAttributesIterator& other)
{
    const TestObjectPhysicalIterator* proper = reinterpret_cast<const TestObjectPhysicalIterator*>(&other);
    assert(proper);
    return _iter == proper->_iter;
}


void
TestObjectPhysicalIterator::operator++()
{
    _iter++;
}
#endif
//==============================================================================

class GFXTester
{
public:
	GFXTester();
	~GFXTester();
	void Update();
	void Render();
	void PageFlip();
   void CreateObject();

   std::list<TestObject*> testObjects;
private:
    Vertex3D* MakeCube(Memory& memory, const Vector3& min, const Vector3& max);

	LMalloc testMemory;
	Display display;
	PixelMap vram;
	ViewPort vp;

	IJoystick			joy1, joy2;
	joystickButtonsF	buttons1, buttons2;

	Vertex3D cubeVertexList[8];
	TriFace cubeFaceList[13];
    Material cubeMaterialList[6];
    Material cube2MaterialList[6];


	RenderCamera camera;
	Vector3 cameraPosition;
	Euler cameraRotation;
};

static GFXTester* tester;

//==========================================================================

static Texture dummyTexture = {"",0,320,0,64,64};
PixelMap* testTexture=0;
PixelMap* test2Texture=0;

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

//=============================================================================

void
AdjustCameraParameters(Vector3& position,Euler& rotation,joystickButtonsF buttons1 )
{
#define ADJUST_ANGLE 1
#define ADJUST_DELTA 0.05

   static int oldButtons=0;
   int buttonsDown = buttons1 & (oldButtons^buttons1);
   oldButtons = buttons1;

   //char  cmd = 0;
	if (buttonsDown & EJ_BUTTONF_A)
       tester->CreateObject();

#if 1
	// change the rotation angles for the cube and the light source
	if (buttons1 & EJ_BUTTONF_E)
		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
	//if (buttons1 & EJ_BUTTONF_A)
	//	rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(ADJUST_ANGLE))));
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
#endif
}

//=============================================================================

#if TEST_TEXTURES

const int BUFFER_SIZE = 100*101;
long unsigned int * buffer[BUFFER_SIZE];

#if !DO_DEBUG_FILE_SYSTEM
#include "testtga.c"
#endif

void
LoadTextures(PixelMap& /*vram*/)
{
#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
   testTexture = new PixelMap(vram,320,0,64,64);
#else
   testTexture = new PixelMap(PixelMap::MEMORY_VIDEO,64,64);
#endif

#if !DO_DEBUG_FILE_SYSTEM
	binistream texturestream((const void*)test,testSize);
#else
	binistream texturestream( "../gfx/test.tga" );
#endif
	LoadTexture(texturestream,*testTexture);

#if !DO_DEBUG_FILE_SYSTEM
    assert(0);
    test2Texture = testTexture;             // just re-use the other one
#else
#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
   test2Texture = new PixelMap(vram,384,0,256,256);
#else
   test2Texture = new PixelMap(PixelMap::MEMORY_VIDEO,256,256);
#endif
    assert(test2Texture);
	binistream texture2stream( "../gfx/test2.tga" );
	LoadTexture(texture2stream,*test2Texture);
#endif

}
#endif

//==============================================================================

Vertex3D*
GFXTester::MakeCube(Memory& memory, const Vector3& min, const Vector3& max)
{
	Vertex3D* vertexList = new (memory) Vertex3D[8];
	assert(ValidPtr(vertexList));

	memcpy(vertexList,cubeVertexList,sizeof(Vertex3D)*8);

	vertexList[0].position = Vector3ToPS(Vector3(min.X(),min.Y(),max.Z()));
	vertexList[1].position = Vector3ToPS(Vector3(max.X(),min.Y(),max.Z()));
	vertexList[2].position = Vector3ToPS(Vector3(min.X(),min.Y(),min.Z()));
	vertexList[3].position = Vector3ToPS(Vector3(max.X(),min.Y(),min.Z()));
	vertexList[4].position = Vector3ToPS(Vector3(min.X(),max.Y(),max.Z()));
	vertexList[5].position = Vector3ToPS(Vector3(max.X(),max.Y(),max.Z()));
	vertexList[6].position = Vector3ToPS(Vector3(max.X(),max.Y(),min.Z()));
	vertexList[7].position = Vector3ToPS(Vector3(min.X(),max.Y(),min.Z()));

	return(vertexList);
}

//=============================================================================

void
GFXTester::CreateObject()
{
   Vector3 max = Vector3(Scalar::Random(SCALAR_CONSTANT(0.1), SCALAR_CONSTANT(0.35)),
                         Scalar::Random(SCALAR_CONSTANT(0.1), SCALAR_CONSTANT(0.35)),
                         Scalar::Random(SCALAR_CONSTANT(0.1), SCALAR_CONSTANT(0.35)));
   Vector3 min = -max;

   //dMatrix3 R;
   Vector3 position;

   if(random_pos)
   {
       position[0] = Scalar::Random(SCALAR_CONSTANT(-1.0),SCALAR_CONSTANT(1.0));
       position[1] = Scalar::Random(SCALAR_CONSTANT(-1.0),SCALAR_CONSTANT(1.0));
       position[2] = Scalar::Random(SCALAR_CONSTANT( 1.0),SCALAR_CONSTANT(2.0));

       //dRFromAxisAndAngle (R,dRandReal()*2.0-1.0,dRandReal()*2.0-1.0,
                           //dRandReal()*2.0-1.0,dRandReal()*10.0-5.0);
   }
#if 0
   else
   {
       dReal maxheight = 0;
       // code to make sure object always falls from above
       for(TestObjectIter iter = testObjects.begin(); iter != testObjects.end(); iter++)
       {
           const dReal *pos = dBodyGetPosition ((*iter)->body);
           if(pos[2] > maxheight) maxheight = pos[2];
       }

       position[0] = Scalar::zero;
       position[1] = Scalar::zero;
       position[2] = Scalar::FromFloat(maxheight+1);

       dRFromAxisAndAngle (R,0,0,1,dRandReal()*10.0-5.0);
   }
#endif

   Euler rotation;

   TestObject* temp = new TestObject(position,rotation,min,max);

   Vertex3D* cubeVertexList = MakeCube(testMemory,min,max);
   temp->object3D = new (testMemory) RenderObject3D (testMemory,8,cubeVertexList,12,cubeFaceList,cubeMaterialList);
   assert(ValidPtr(temp->object3D));
   testObjects.push_back(temp);
}

//==============================================================================

extern void test_event_loop();
extern int _halWindowWidth, _halWindowHeight;

GFXTester::GFXTester() :
	testMemory(HALLmalloc,200000 MEMORY_NAMED( COMMA "GFX test Memory" )),
	display( 2, 0,0,320, 240, testMemory ),

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
// kts change this to use the video memory allocated in Display
	vram(PixelMap::MEMORY_VIDEO,Display::VRAMWidth, Display::VRAMHeight),
#else
	vram(PixelMap::MEMORY_VIDEO,256, 256),
#endif
	vp(display,4000,Scalar( 320, 0 ), Scalar( 240, 0 ), testMemory, 2 )
	,camera(vp)

{
#if TEST_TEXTURES                                                              
	LoadTextures(vram);
#endif

	joy1 = JoystickNew(EJW_JOYSTICK1);
	joy2 = JoystickNew(EJW_JOYSTICK2);

	display.SetBackgroundColor(Color(60,60,60));
	#define SIZE 0.25
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

    std::cout << "vram& = "<< (void*)&vram << std::endl;
	cubeMaterialList[0] = Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,test2Texture);                   // blue
	cubeMaterialList[1] = Material(Color( 40,168, 40),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,test2Texture);                   // green
	cubeMaterialList[2] = Material(Color( 40,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,test2Texture);                   // cyan
	cubeMaterialList[3] = Material(Color(168, 40, 40),Material::FLAT_SHADED|Material::TEXTURE_MAPPED|Material::LIGHTING_LIT,dummyTexture,test2Texture);                     // red
	cubeMaterialList[4] = Material(Color(128,128,128),Material::FLAT_SHADED|Material::TEXTURE_MAPPED|Material::LIGHTING_LIT,dummyTexture,test2Texture);                        // yellow
	cubeMaterialList[5] = Material(Color(168, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,test2Texture);                        // pink

	cube2MaterialList[0] = Material(Color( 40, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,testTexture);                   // blue
	cube2MaterialList[1] = Material(Color( 40,168, 40),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,testTexture);                   // green
	cube2MaterialList[2] = Material(Color( 40,168,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,testTexture);                   // cyan
	cube2MaterialList[3] = Material(Color(168, 40, 40),Material::FLAT_SHADED|Material::TEXTURE_MAPPED|Material::LIGHTING_LIT,dummyTexture,testTexture);                     // red
	cube2MaterialList[4] = Material(Color(128,128,128),Material::FLAT_SHADED|Material::TEXTURE_MAPPED|Material::LIGHTING_LIT,dummyTexture,testTexture);                        // yellow
	cube2MaterialList[5] = Material(Color(168, 40,168),Material::FLAT_SHADED|Material::SOLID_COLOR|Material::LIGHTING_LIT,dummyTexture,testTexture);                        // pink

   //Vector3 min(SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT(-SIZE),SCALAR_CONSTANT(-SIZE));
   //Vector3 max(SCALAR_CONSTANT(SIZE),SCALAR_CONSTANT(SIZE),SCALAR_CONSTANT(SIZE));

	//cameraPosition = Vector3( Scalar::zero, Scalar::zero, Scalar::zero );
	cameraPosition = Vector3( SCALAR_CONSTANT(0), SCALAR_CONSTANT(0), SCALAR_CONSTANT(5) );
//      Vector3 cameraPosition( SCALAR_CONSTANT(-4), SCALAR_CONSTANT(-4), SCALAR_CONSTANT(24) );
	cameraRotation = Euler(Angle::Degree(SCALAR_CONSTANT(90)),Angle::Degree(SCALAR_CONSTANT(0)),Angle::zero);
//      Euler cameraRotation(Angle::Degree(SCALAR_CONSTANT(7.998046)),Angle::Degree(SCALAR_CONSTANT(187.998046)),Angle::zero);
}

//=============================================================================

GFXTester::~GFXTester()
{
   for(TestObjectIter iter = testObjects.begin(); iter != testObjects.end(); iter++)
   {
      MEMORY_DELETE( testMemory, (*iter)->object3D,RenderObject3D);
   }
	JoystickDelete(joy1);
	JoystickDelete(joy2);
}

//==============================================================================
// this is called by dSpaceCollide when two objects in space are
// potentially colliding.

static void nearCallback (void* /*data*/, ode::dGeomID o1, ode::dGeomID o2)
{
  int i;
  // if (o1->body && o2->body) return;

  // exit without doing anything if the two bodies are connected by a joint
  ode::dBodyID b1 = ode::dGeomGetBody(o1);
  ode::dBodyID b2 = ode::dGeomGetBody(o2);
  if (b1 && b2 && ode::dAreConnected (b1,b2)) return;

  ode::dContact contact[4];			// up to 4 contacts per box-box
  for (i=0; i<4; i++) {
    contact[i].surface.mode = ode::dContactBounce | ode::dContactSoftCFM;
    contact[i].surface.mu = ode::dInfinity;
    contact[i].surface.mu2 = 0;
    contact[i].surface.bounce = 0.1;
    contact[i].surface.bounce_vel = 0.1;
    contact[i].surface.soft_cfm = 0.01;
  }
  if (int numc = ode::dCollide (o1,o2,4,&contact[0].geom,sizeof(ode::dContact))) {
    //dMatrix3 RI;
    //dRSetIdentity (RI);
    //const dReal ss[3] = {0.02,0.02,0.02};
    for (i=0; i<numc; i++) {
      ode::dJointID c = ode::dJointCreateContact (odeWorld(),contactgroup,contact+i);
      ode::dJointAttach (c,b1,b2);
      //if (show_contacts) dsDrawBox (contact[i].geom.pos,RI,ss);
    }
  }
}

//=============================================================================

void
GFXTester::Update()
{
	buttons1 = JoystickGetButtonsF(joy1);
	buttons2 = JoystickGetButtonsF(joy2);

//	// update camera
	AdjustCameraParameters(cameraPosition,cameraRotation,buttons1);

   dSpaceCollide (space,0,&nearCallback);
   dWorldStep (odeWorld(),0.05);

 // remove all contact joints
   dJointGroupEmpty (contactgroup);

   for(TestObjectIter iter = testObjects.begin(); iter != testObjects.end(); iter++)
   {
       TestObject& testObject = *(*iter);
       testObject._physical.Update();
   }
}

//============================================================================

void
GFXTester::Render()
{
	display.RenderBegin();
	Matrix34 camRot(cameraRotation,Vector3::zero);

	Matrix34 camPos(cameraPosition);
	Matrix34 camMat = camPos * camRot;

	camera.SetPosition(camMat);

	camera.SetFog(Color(60,60,60),SCALAR_CONSTANT(10),SCALAR_CONSTANT(18));
	camera.SetAmbientColor(Color(128,128,128));
	camera.SetDirectionalLight(0,Vector3(SCALAR_CONSTANT(1),SCALAR_CONSTANT(0),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(1,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(1),SCALAR_CONSTANT(0)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1),PS_SCALAR_CONSTANT12(  0)));
	camera.SetDirectionalLight(2,Vector3(SCALAR_CONSTANT(0),SCALAR_CONSTANT(0),SCALAR_CONSTANT(1)),Color48(PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  0),PS_SCALAR_CONSTANT12(  1)));
		
    // actually draw them
		vp.Clear();
		camera.RenderBegin();

      for(TestObjectIter iter = testObjects.begin();iter != testObjects.end(); iter++)
      {
         TestObject* o = (*iter);
         camera.RenderObject(*o->object3D,o->_physical.Matrix());
      }
		camera.RenderEnd();

		vp.Render();
	display.RenderEnd();
}

//============================================================================

void
GFXTester::PageFlip()
{
	display.PageFlip();
}

//==============================================================================

void
TestGFXInit()
{
	printf("Test gfx init\n");
   space = ode::dHashSpaceCreate (0);
   contactgroup = ode::dJointGroupCreate (0);
   ode::dWorldSetGravity (odeWorld(),0,0,-0.5);
   ode::dWorldSetCFM (odeWorld(),1e-5);
   ode::dCreatePlane (space,0,0,1,0);

	tester = new (HALLmalloc) GFXTester;
	assert(ValidPtr(tester));
}

void
TestGFXUnInit()
{
	printf("Test gfx uninit\n");
   ode::dJointGroupDestroy (contactgroup);
   ode::dSpaceDestroy (space);
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

//=============================================================================

void
PIGSMain( int argc, char* argv[] )
{
    (void)argc;
    (void)argv;
	std::cout << "PhysicsTest:" << std::endl;

	printf("Test gfx\n");
	TestGFXInit();
	printf("Test gfx init done\n");

	for (;;)
	{
		TestGFXLoop();
		TestGFXPageFlip();
	}

	printf("shutting down\n");
	TestGFXUnInit();
	printf("gfx test complete\n");

   std::cout << "someone should write some physics tests:-)" << std::endl;
	std::cout << "physicstest Done:" << std::endl;
	PIGSExit();
}

//==============================================================================


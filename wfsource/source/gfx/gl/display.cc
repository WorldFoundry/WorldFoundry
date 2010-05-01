//=============================================================================
// gfx/gl/display.cc: display hardware abstraction class, windows openGL specific code
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
// Description: The Display class encapsulates data and behavior for a single
//       hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#include <hal/hal.h>
#include <GL/gl.h>

#include <memory/memory.hp>
#include <gfx/pixelmap.hp>
#include <gfx/rendobj3.hp>
extern bool bFullScreen;
extern int _halWindowWidth;
extern int _halWindowHeight;

extern RendererVariables globalRendererVariables;

//#       include <gl/glaux.h>
#       include <gfx/gl/wfprim.h>

#include <math.h>

// Keep track of windows changing width and height
GLfloat windowXPos;
GLfloat windowYPos;
GLfloat windowWidth;
GLfloat windowHeight;
int wfWindowWidth = 640;
int wfWindowHeight = 480;


#if defined(__WIN__)
#include <Mmsystem.h>
#include <time.h>
#include "wgl.cc"
#endif
#if defined(__LINUX__)
#include "mesa.cc" 
#include <sys/time.h>
#include <unistd.h>
#endif

//==============================================================================

#if 0
#include <GL/glut.h>

void
TestGL2(void)
{
   glDisable( GL_TEXTURE_2D );
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0,1.0,1.0,1.0 };
   GLfloat white_light[] = { 1.0,1.0,1.0,1.0 };
   GLfloat black_light[] = { 0.0,0.0,0.0,1.0 };
   GLfloat mat_ambient_color[] = { 0.8,0.8,0.2,1.0 };
   GLfloat mat_diffuse[] = { 0.1,0.5, 0.8, 1.0 };
   glClearColor(0.0,0.0,0.0,0.0);
   glShadeModel(GL_SMOOTH);
   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT,white_light);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_AMBIENT, white_light);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
   glEnable(GL_LIGHTING);
   glDisable(GL_LIGHT0);
   glDisable(GL_LIGHT1);
   glDisable(GL_LIGHT2);
   glDisable(GL_LIGHT3);
   glDisable(GL_LIGHT4);
   glDisable(GL_LIGHT5);
   glDisable(GL_LIGHT6);
   glDisable(GL_LIGHT7);
   glEnable(GL_DEPTH_TEST);
   int w = 320;
   int h = 200;
   glViewport(0, 0, (GLsizei) w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if(w<=h)
      glOrtho(-1.5,1.5,-1.5*(GLfloat)h/(GLfloat)w,1.5*(GLfloat)h/(GLfloat)w, -10.0, 10.0);
   else
      glOrtho(-1.5*(GLfloat)w/(GLfloat)h,1.5*(GLfloat)w/(GLfloat)h, -1.5, 1.5, -10.0, 10.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   while(1)
   {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glutSolidSphere(1.0,20,16);

      float zOffset = 15.0;
   #if 1
      glBegin(GL_TRIANGLES);
      glVertex3f( 0.9, -0.9, -10.0 + zOffset);
      glVertex3f( 0.9,  0.9, -10.0 + zOffset);
      glVertex3f(-0.9,  0.0, -10.0 + zOffset);
      glVertex3f(-0.9, -0.9, -20.0 + zOffset);
      glVertex3f(-0.9,  0.9, -20.0 + zOffset);
      glVertex3f( 0.9,  0.0, -5.0 + zOffset);
      glEnd();
   #endif
      glFlush();

      glXSwapBuffers(halDisplay.mainDisplay, halDisplay.win);
      AssertGLOK();
   }
}


void
display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glColor3f(0.0, 1.0, 0.0);
   glutSolidSphere(1.0,20,16);

   float zOffset = 15.0;
#if 1
   glBegin(GL_TRIANGLES);
   glColor3f(1.0, 1.0, 1.0);
   glVertex3f( 0.9, -0.9, -10.0 + zOffset);
   glVertex3f( 0.9,  0.9, -10.0 + zOffset);
   glVertex3f(-0.9,  0.0, -10.0 + zOffset);
   glColor3f(0.0, 1.0, 0.0);
   glVertex3f(-0.9, -0.9, -20.0 + zOffset);
   glVertex3f(-0.9,  0.9, -20.0 + zOffset);
   glVertex3f( 0.9,  0.0, -5.0 + zOffset);
   glEnd();
#endif
   glFlush();
}

//==============================================================================

void
reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei)h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if(w<=h)
      glOrtho(-1.5,1.5,-1.5*(GLfloat)h/(GLfloat)w,1.5*(GLfloat)h/(GLfloat)w, -10.0, 10.0);
   else
      glOrtho(-1.5*(GLfloat)w/(GLfloat)h,1.5*(GLfloat)w/(GLfloat)h, -1.5, 1.5, -10.0, 10.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}

//==============================================================================

void
TestGL(void)
{

   int argc=1;
   char* argv[] = {"program"};
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(500,500);
   glutInitWindowPosition(100,100);
   glutCreateWindow("testGL");
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0,1.0,1.0,1.0 };
   GLfloat white_light[] = { 1.0,1.0,1.0,1.0 };
   GLfloat mat_ambient_color[] = { 0.8,0.8,0.2,1.0 };
   GLfloat mat_diffuse[] = { 0.1,0.5, 0.8, 1.0 };
   glClearColor(0.0,0.0,0.0,0.0);
   glShadeModel(GL_SMOOTH);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutMainLoop();
}
#endif

//==============================================================================

void
WFInitGL()
{
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	AssertGLOK();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	AssertGLOK();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	AssertGLOK();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	AssertGLOK();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	AssertGLOK();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glShadeModel( GL_FLAT );
    glShadeModel( GL_SMOOTH ); 
    AssertGLOK();
    glClearColor( 0.5, 0.5, 0.5, 1.0 );
    AssertGLOK();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    AssertGLOK();

    glEnable(GL_BLEND);
    AssertGLOK();
#if !defined( RENDERER_PIPELINE_GL )
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    AssertGLOK();
#endif

    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    AssertGLOK();
//    glClearIndex( Black );

    glMatrixMode( GL_MODELVIEW );
    AssertGLOK();
    glLoadIdentity();
    AssertGLOK();

    //glCullFace( GL_BACK );
    //glEnable( GL_CULL_FACE );

    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    //if(h == 0)
    //	h = 1;

    // Set the viewport to be the entire window
    glViewport(0, 0, wfWindowWidth, wfWindowHeight);
    AssertGLOK();

    // Keep the square square, this time, save calculated
    // width and height for later use
//     if (w <= h)
//     {
//         windowHeight = 250.0f*h/w;
//         windowWidth = 250.0f;
//     }
//     else
//     {
//         windowWidth = 250.0f*w/h;
//         windowHeight = 250.0f;
//     }
    // Set the clipping volume

    
    glMatrixMode( GL_PROJECTION );
    AssertGLOK();
    glLoadIdentity();
    
    AssertGLOK();
#if defined( RENDERER_PIPELINE_GL )
    //float fAspect = float(wfWindowWidth)/float(wfWindowHeight);
    float fAspect = 1.0;                           // kts I am correcting for this elsewhere, eventually in the case of PIPELINE_GL this will need to be changed
    gluPerspective(60.0f,fAspect,1.0,1000.0f);
#else
#if defined ( GFX_ZBUFFER )
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5f, 1000.0f);
#else /* GFX_ZBUFFER */
    glOrtho(-320.0f/2, 320/2, -320.0f/2, 320/2, 1.0f, -1.0f); 
#endif /* GFX_ZBUFFER */
#endif
    AssertGLOK();

    glMatrixMode( GL_MODELVIEW );
    AssertGLOK();

}
//==============================================================================

Display::Display(int orderTableSize, int xPos, int yPos, int xSize, int ySize, Memory& memory,bool /*interlace*/) :
_drawPage(0),
#if defined(USE_ORDER_TABLES)
_constructionOrderTableIndex(0),
_renderOrderTableIndex(1),
#endif
_xPos(xPos),
_yPos(yPos),
_xSize(xSize),
_ySize(ySize),
_memory(memory)
{

    _memory.Validate();
    if(!InitWindow(xPos, yPos, _halWindowWidth, _halWindowHeight ))
    {
        printf("Display::Display:doInit Failed!\n");
        sys_exit(1);
    }
    AssertGLOK();

	WFInitGL();

    assert(orderTableSize > 0);
#if defined(USE_ORDER_TABLES)
    for(int index=0;index<ORDER_TABLES;index++)
    {
        _orderTable[index] = new (_memory) OrderTable(orderTableSize,_memory);
        assert(ValidPtr(_orderTable[index]));
    }
#endif

    // set up GL 
   //glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,1);
   //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    _drawPage = 0;
    ResetTime();

#if 0
#pragma message ("KTS " __FILE__ ": temp test code")
    for(int y=0; y<VRAM_HEIGHT; ++y)
    {
        for(int x=0; x<VRAM_WIDTH; ++x)
        {
#if SIXTEEN_BIT_VRAM
            vram[ y ][ x ] = 0x7fff;
#else
            vram[ y ][ x ][0] = 128;
            vram[ y ][ x ][1] = 128;
            vram[ y ][ x ][2] = 255;
            vram[ y ][ x ][3] = 255;
#endif
        }
    }
#endif

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
    _videoMemory = new (HALLmalloc) PixelMap( PixelMap::MEMORY_VIDEO, VRAMWidth, VRAMHeight );
    assert( ValidPtr( _videoMemory ) );
#else

// do nothing
#endif
}

//==============================================================================

Display::~Display()
{
    Validate();
    if(_drawPage == 0)
        PageFlip();

    PageFlip();

#if defined(USE_ORDER_TABLES)
    for(int index=ORDER_TABLES-1;index>= 0;index--)
        _memory.Free(_orderTable[index],sizeof(OrderTable));
#endif

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
    delete _videoMemory;
#else

// do nothing
#endif
}

//============================================================================

#if defined(__LINUX__)
inline Scalar
ConvertTimeToScalar(const struct timeval&  tv)
{
    int16 whole = tv.tv_sec;
    uint16 frac;

    frac = uint16(float(tv.tv_usec)/(15.2587890625));
    assert(tv.tv_sec < USHRT_MAX);
    whole = tv.tv_sec;
    return(Scalar(whole,frac));
}
#endif

//==============================================================================

void
Display::ResetTime()                    // used to reset delta timer for PageFlip
{
    //_clockLastTime = timeGetTime();  	//clock();

#if defined(__WIN__)
    _clockLastTime = timeGetTime();                 //clock();
#elif defined(__LINUX__)
    struct timeval tv;
    gettimeofday(&tv,NULL);
    _clockLastTime = tv;                
#else
#error platform not supported
#endif
}

//============================================================================

void
Display::RenderBegin()
{
   Validate();
   AssertGLOK();
   AssertMsg( _drawPage == 0 || _drawPage == 1, "_drawPage = " << _drawPage );
   glClearColor( _backgroundColorRed, _backgroundColorGreen, _backgroundColorBlue, 1.0 );
   AssertGLOK();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear the window with current clearing color
   AssertGLOK();
#if defined(RENDERER_PIPELINE_GL)
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHT1);
   glEnable(GL_LIGHT2);
   glEnable(GL_NORMALIZE);
   glEnable(GL_FOG);

   GLfloat lightWhite[] = {
       1.0, 1.0, 1.0, 1.0
   };


   GLfloat lightBlack[] = {
       0.0, 0.0, 0.0, 0.0
   };
   glMaterialfv(GL_FRONT,GL_AMBIENT,lightWhite);
   glMaterialfv(GL_FRONT,GL_DIFFUSE,lightWhite);
   glMaterialfv(GL_FRONT,GL_SPECULAR,lightBlack);
   AssertGLOK();
#else
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
   glDisable(GL_LIGHTING);
#endif

   AssertGLOK();
   glEnable( GL_TEXTURE_2D );
   AssertGLOK();

#if 0
   static GLfloat cameraZ = -5.0;
   static GLfloat zOffset = 0;
   //cameraZ += 0.1;
   //zOffset += 0.1;
   //cout << "cz: " << cameraZ << ", zo:" << zOffset << endl;

   glMatrixMode (GL_MODELVIEW);
   glLoadIdentity();
   //glTranslatef(0.0,0.0,cameraZ);

    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);

   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0,1.0,1.0,1.0 };
   GLfloat white_light[] = { 1.0,1.0,1.0,1.0 };
   GLfloat mat_ambient_color[] = { 0.8,0.8,0.2,1.0 };
   GLfloat mat_diffuse[] = { 0.1,0.5, 0.8, 1.0 };
   glClearColor(0.0,0.0,0.0,0.0);
   glShadeModel(GL_SMOOTH);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glLightfv(GL_LIGHT0, GL_POSITION, light_position);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);

    glBegin(GL_TRIANGLES);
    glColor3f(1.0, 1.0, 1.0);
    glVertex3f( 0.9, -0.9, -10.0 + zOffset);
    glVertex3f( 0.9,  0.9, -10.0 + zOffset);
    glVertex3f(-0.9,  0.0, -10.0 + zOffset);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(-0.9, -0.9, -20.0 + zOffset);
    glVertex3f(-0.9,  0.9, -20.0 + zOffset);
    glVertex3f( 0.9,  0.0, -5.0 + zOffset);
    glEnd();
#endif
    glLoadIdentity ();
}

//==============================================================================

void
Display::RenderEnd()
{
}

//==============================================================================

extern bool	windowActive;		// Window windowActive Flag Set To TRUE By Default


Scalar
Display::PageFlip()
{
#if 0

    // event_loop( dpy );
    XEvent event;

    while(XCheckMaskEvent(dpy, 0xffffffff,&event))
    {
   //while (1) {
      //XNextEvent( dpy, &event );

        switch(event.type)
        {
            case Expose:
                redraw( dpy, event.xany.window );
                break;
            case ConfigureNotify:
                resize( event.xconfigure.width, event.xconfigure.height );
                break;
        }
    }
#endif

#if defined(__WIN__)
	MSG		msg;									// Windows Message Structure
		  
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
	{
		if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
		{
            printf("Display::PageFlip: gl\\display.cc calling sys_exit\n");
			KillGLWindow();									// Kill The Window
			sys_exit(1);
		}
		else									// If Not, Deal With Window Messages
		{
			TranslateMessage(&msg);				// Translate The Message
			DispatchMessage(&msg);				// Dispatch The Message
		}
	}
	{
		// Watch For ESC Key 
		if (keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
		{
            printf("Display::PageFlip: gl\\display.cc calling sys_exit\n");
			KillGLWindow();									// Kill The Window
			sys_exit(1);
		}

		if (keys[VK_F1])						// Is F1 Being Pressed?
		{
			keys[VK_F1]=FALSE;					// If So Make Key FALSE
			KillGLWindow();						// Kill Our Current Window
			fullscreen=!fullscreen;				// Toggle Fullscreen / Windowed Mode
			// Recreate Our OpenGL Window
			if (!CreateGLWindow("WorldFoundry",_xPos, _yPos, _halWindowWidth, _halWindowHeight,16,fullscreen))
			{
				return 0;						// Quit If Window Was Not Created
			}
			WFInitGL();
		}
	}
#endif
#if defined(__LINUX__)
    XEventLoop(); 
#endif

    Validate();

//FntPrint("\nWorld Foundry Display: page %d\n",_drawPage);
#if 0
        // kts test code
    glColor3f( 1.0, 0.0, 1.0 );
    AssertGLOK();
    glBegin( GL_TRIANGLES );
    glVertex3f(  100.0,  100.0,  -5.0);
    glVertex3f( -100.0,  100.0,  -5.0);
    glVertex3f( -100.0, -100.0,  -5.0);
    glEnd();
    AssertGLOK();

          //glDisable( GL_TEXTURE_2D );

    glBegin( GL_TRIANGLES );
          //glColor3f( 1.0, 1.0, 0.0 );
    glColor3ub( 200, 200, 200 );

    glVertex2i( 200, 200 );
    glVertex2i( 0, 200 );
    glVertex2i( 0, -200 );
    glEnd();
    AssertGLOK();
        // end test code
#endif

#if defined(RENDERER_PIPELINE_SOFTWARE)
#if defined(USE_ORDER_TABLES)
    SetConstructionOrderTableIndex(_drawPage);
    SetRenderOrderTableIndex(1-_drawPage);

    _orderTable[GetRenderOrderTableIndex()]->Render();
    _drawPage ^= 1;
    _orderTable[GetConstructionOrderTableIndex()]->Clear();
#else // USE_ORDER_TABLES
#error now what?
    assert(0);
#endif
#elif defined(RENDERER_PIPELINE_GL) // defined(RENDERER_PIPELINE_SOFTWARE)

#if 0
    static float xRot;
    xRot += 1.0f;
    static float yRot;
    yRot += 1.0f;
    glPushMatrix();
    AssertGLOK();
    glRotatef(xRot,1.0f,0.0f,0.0f);
    AssertGLOK();
    glRotatef(yRot,0.0f,1.0f,0.0f);
    AssertGLOK();

    GLfloat x,y,z,angle;
    glClearColor(0.0f, 0.0f,0.0f,1.0f);
    AssertGLOK();
    glColor3f(0.0f,1.0f,0.0f);
    AssertGLOK();

    glClear(GL_COLOR_BUFFER_BIT);
    AssertGLOK();
    glDisable( GL_TEXTURE_2D );
    AssertGLOK();
    glBegin(GL_TRIANGLES);

    glColor3ub( 128, 128, 0 );
    z = -50.0f;
    for(angle=0.0f; angle <= (2.0f*3.1415)*3.0f; angle += 0.1f)
    {
        x = 50.0f*sin(angle);
        y = 50.0f*cos(angle);
        glVertex3f(0.0f+200.0f,0.0f,0.0f);
        glVertex3f(x+200.0f,y,z);
        x = 50.0f*sin(angle+10.0);
        y = 50.0f*cos(angle+10.0);
        glVertex3f(x+200.0f,y,z);
        z += 0.5f;
    }
    glEnd();
    AssertGLOK();
    glPopMatrix();

#endif // 0


#else
#error renderer pipeline not defined!
#endif

    glFlush();
    AssertGLOK();

#if defined(__WIN__)
    // Call function to swap the buffers
	SwapBuffers(hDC);					// Swap Buffers (Double Buffering)
#elif defined(__LINUX__)
    glXSwapBuffers(halDisplay.mainDisplay, halDisplay.win);
    AssertGLOK();

         // glFinish();
#else
#error platform not defined
#endif


//	if(!wglMakeCurrent(hardwaredevicecontext,NULL))
//	{
//		assert(0);
//	}
//	wglDeleteContext(hRC);
//	hRC = 0;

    // now calc how long it has been since last frame
#if defined(__WIN__)
    unsigned long now = timeGetTime();                  //clock();
    unsigned long delta = now - _clockLastTime;
    _clockLastTime = now;

    AssertMsg(delta/CLOCKS_PER_SEC < 65536, 
              "delta/CLOCKS_PER_SEC = " << delta/CLOCKS_PER_SEC << ", delta = " << delta << ", CLOCKS_PER_SEC = " << CLOCKS_PER_SEC);
    Scalar whole(short(delta/CLOCKS_PER_SEC),0);
    Scalar frac(short(delta%CLOCKS_PER_SEC),0);
    frac /= SCALAR_CONSTANT(CLOCKS_PER_SEC);
    return(whole+frac);
#elif defined(__LINUX__)
    struct timeval tv;
    gettimeofday(&tv,NULL);

    struct timeval deltatime;
    deltatime.tv_usec = tv.tv_usec - _clockLastTime.tv_usec;
    deltatime.tv_sec = tv.tv_sec - _clockLastTime.tv_sec;
    assert(deltatime.tv_sec < 5);               // if more than 5 seconds something is really wrong
    int tempCounter = 0;
    while(deltatime.tv_usec < 0)
    {
        deltatime.tv_usec += 1000000;
        deltatime.tv_sec--;
        tempCounter++;
    }

    assert(tempCounter < 5);

    Scalar delta = ConvertTimeToScalar(deltatime);


    if(delta > SCALAR_CONSTANT(1.0/5.0))            // if delta less than 1/5 of a second, prop it up
    {
        std::cout << "delta too large: " << delta << std::endl;
        std::cout << "timeofday:" << tv.tv_sec << ":" << tv.tv_usec << std::endl;
        std::cout << "lasttimeofday: " << _clockLastTime.tv_sec << ":" << _clockLastTime.tv_usec << std::endl;
        std::cout << "deltatime:" << deltatime.tv_sec << ":" << deltatime.tv_usec << std::endl;
        std::cout << "delta: " << delta << std::endl;
        delta = SCALAR_CONSTANT(1.0/5.0);
    }

    // don't allow framerate to exceed 1200 fps ;-)
    if(delta < Scalar(SCALAR_CONSTANT(1.0/1200)))
    {
        delta = Scalar(SCALAR_CONSTANT(1.0/1200));
    }

    _clockLastTime = tv;
    return(delta);
#else
#error platform not defined
#endif

}

//============================================================================

#if defined(USE_ORDER_TABLES)

inline void
SetTexture(const PixelMap& texturePixelMap)
{
#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
#else
    texturePixelMap.SetGLTexture();
#endif
}

void 
CalcAndSetUV(unsigned short tpage, unsigned char uin, unsigned char vin, const PixelMap& texturePixelMap) 
{
    ulong u(uin+DecodeTPageX(tpage)); 
    ulong v(vin+DecodeTPageY(tpage)); 

#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
    float uResult(float(u)/VRAM_WIDTHF);                            
    float vResult(float(v)/VRAM_HEIGHTF);                           

    // kts temp test code
    //texturePixelMap.SetGLTexture();
#else
    float uResult(float(u)/texturePixelMap.GetBaseXSize());                            
    float vResult(float(v)/texturePixelMap.GetBaseYSize());
#endif
    glTexCoord2f(uResult, vResult);                         
}


//==============================================================================

inline void
GL_3D_VERTEX(const Point3D& point)                                                                         
{                                                                                              
    float fx = float(point.x) / 65536.0;                                                         
    float fy = float(point.y) / 65536.0;                                                         
    float fz = float(point.z) / 65536.0;
    //cout.setf(ios::fixed,ios::basefield);
    //cout << "x:" << x << ", y:" << y << ", wx:" << wx << ", wy:" << wy << ", px:" << float(point.x)/65536.0 << ", py:" << float(point.y)/65536.0 << ", pz:" << point.z << ", fz:" << fz << endl; 
    glVertex4f(fx,-fy,1.0,fz);
}

//==============================================================================

void
DrawOTag( ORDER_TABLE_ENTRY* __orderTable )
{
    Primitive* _orderTable = (Primitive*)__orderTable;

    assert( _orderTable );
    Primitive* _orderTableEnd = _orderTable;

    assert( CODE_NOP == 0 );

    // kts temp
//   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glClear( GL_COLOR_BUFFER_BIT );

    //glVertex2f( (float)x, (float)-y );

// #define checkImageWidth 64
// #define checkImageHeight 64
// extern GLubyte checkImage[checkImageHeight][checkImageWidth][4];


    for(; !isendprim( _orderTable ); _orderTable = (Primitive*)nextPrim( _orderTable ))
    {
        ValidatePtr(_orderTable);
        Primitive* pTag = _orderTable;
        uint8 code = pTag->base.code;
#if 0
        std::cout << "otable code = " << int(_orderTable->code) << std::endl;
        std::cout << "code = " << int(code) << std::endl;
#endif
        //cout << "new poly:" << endl;
        if(code)
        {
            switch(code)
            {
                case CODE_POLY_F3:
                    {
                        glDisable( GL_TEXTURE_2D );
                        AssertGLOK();
                        glBegin( GL_TRIANGLES );
//					assert(0);
                        POLY_F3& pPoly = _orderTable->f3;

                        glColor3ub( pPoly.r0, pPoly.g0, pPoly.b0 );
                    //glColor3ub( rand() % 255, rand() % 255, rand() % 255 );
#if defined(GFX_ZBUFFER)
                        GL_3D_VERTEX(pPoly.point0);
                        GL_3D_VERTEX(pPoly.point1);
                        GL_3D_VERTEX(pPoly.point2);
#else /* defined(GFX_ZBUFFER) */
                        glVertex2i( pPoly.x0, -pPoly.y0 );
                        glVertex2i( pPoly.x1, -pPoly.y1 );
                        glVertex2i( pPoly.x2, -pPoly.y2 );
#endif /* defined(GFX_ZBUFFER) */
                        glEnd();
                        AssertGLOK();
                        glEnable( GL_TEXTURE_2D );
                        AssertGLOK();
                        break;
                    }

                case CODE_POLY_FT3:
                    {
//					assert(0);
                        POLY_FT3& pPoly = _orderTable->ft3;
                        assert(pPoly.pPixelMap);
                        SetTexture(*pPoly.pPixelMap);
                        glBegin( GL_TRIANGLES );

                    //glColor3ub( rand() % 255, rand() % 255, rand() % 255 );
                    //assert( theTexture );
                    //glCallList( theTexture );
                        glColor3ub( 255, 0, 0 );
                        assert(pPoly.pPixelMap);
#if defined( GFX_ZBUFFER )
                        CalcAndSetUV(pPoly.tpage, pPoly.u0,pPoly.v0,*pPoly.pPixelMap);
                        GL_3D_VERTEX(pPoly.point0);
                        CalcAndSetUV(pPoly.tpage, pPoly.u1,pPoly.v1,*pPoly.pPixelMap);
                        GL_3D_VERTEX(pPoly.point1);
                        CalcAndSetUV(pPoly.tpage, pPoly.u2,pPoly.v2,*pPoly.pPixelMap);
                        GL_3D_VERTEX(pPoly.point2);
#else /* GFX_ZBUFFER */
                        CalcAndSetUV(pPoly.tpage, pPoly.u0,pPoly.v0,*pPoly.pPixelMap);
                        glVertex2i( pPoly.x0, -pPoly.y0 );
                        CalcAndSetUV(pPoly.tpage, pPoly.u1,pPoly.v1,*pPoly.pPixelMap);
                        glVertex2i( pPoly.x1, -pPoly.y1 );
                        CalcAndSetUV(pPoly.tpage, pPoly.u2,pPoly.v2,*pPoly.pPixelMap);
                        glVertex2i( pPoly.x2, -pPoly.y2 );
#endif /* GFX_ZBUFFER */
                        glEnd();
                        AssertGLOK();
                        break;
                    }

                case CODE_POLY_G3:
                    {
                        glDisable( GL_TEXTURE_2D );
                        AssertGLOK();
                        glBegin( GL_TRIANGLES );
                        POLY_G3& pPoly = _orderTable->g3;

#if defined ( GFX_ZBUFFER )
//					glVertex2i( pPoly.x0, - (pPoly.y0) );
                        glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
                        GL_3D_VERTEX(pPoly.point0);
                        glColor3f( float(pPoly.r1)/128, float(pPoly.g1)/128, float(pPoly.b1)/128 );
                        GL_3D_VERTEX(pPoly.point1);
                        glColor3f( float(pPoly.r2)/128, float(pPoly.g2)/128, float(pPoly.b2)/128 );
                        GL_3D_VERTEX(pPoly.point2);
#else /* GFX_ZBUFFER */
                        glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
                        glVertex2i( pPoly.x0, -pPoly.y0 );
                        glColor3f( float(pPoly.r1)/128, float(pPoly.g1)/128, float(pPoly.b1)/128 );
                        glVertex2i( pPoly.x1, -pPoly.y1 );
                        glColor3f( float(pPoly.r2)/128, float(pPoly.g2)/128, float(pPoly.b2)/128 );
                        glVertex2i( pPoly.x2, -pPoly.y2 );
#endif /* GFX_ZBUFFER */
                        glEnd();
                        AssertGLOK();
                        glEnable( GL_TEXTURE_2D );
                        AssertGLOK();
                        break;
                    }

                case CODE_POLY_GT3:
                    {
                        POLY_GT3& pPoly = _orderTable->gt3;
                        assert(pPoly.pPixelMap);
                        SetTexture(*pPoly.pPixelMap);
                        glBegin( GL_TRIANGLES );
                        CalcAndSetUV(pPoly.tpage, pPoly.u0,pPoly.v0,*pPoly.pPixelMap);
#if defined ( GFX_ZBUFFER )
                        glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
                        GL_3D_VERTEX(pPoly.point0);
                        CalcAndSetUV(pPoly.tpage, pPoly.u1,pPoly.v1,*pPoly.pPixelMap);
                        glColor3f( float(pPoly.r1)/128, float(pPoly.g1)/128, float(pPoly.b1)/128 );
                        GL_3D_VERTEX(pPoly.point1);
                        CalcAndSetUV(pPoly.tpage, pPoly.u2,pPoly.v2,*pPoly.pPixelMap);
                        glColor3f( float(pPoly.r2)/128, float(pPoly.g2)/128, float(pPoly.b2)/128 );
                        GL_3D_VERTEX(pPoly.point2);
#else /* GFX_ZBUFFER */
                        glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
                        glVertex2i( pPoly.x0, -pPoly.y0 );
                        CalcAndSetUV(pPoly.tpage, pPoly.u1,pPoly.v1,*pPoly.pPixelMap);
                        glColor3f( float(pPoly.r1)/128, float(pPoly.g1)/128, float(pPoly.b1)/128 );
                        glVertex2i( pPoly.x1, -pPoly.y1 );
                        CalcAndSetUV(pPoly.tpage, pPoly.u2,pPoly.v2,*pPoly.pPixelMap);
                        glColor3f( float(pPoly.r2)/128, float(pPoly.g2)/128, float(pPoly.b2)/128 );
                        glVertex2i( pPoly.x2, -pPoly.y2 );
#endif /* GFX_ZBUFFER */
                        glEnd();
                        AssertGLOK();
                        break;
                    }

                case CODE_SPRT_16:
                    {
                        glDisable( GL_TEXTURE_2D );
                        AssertGLOK();
                        glBegin( GL_TRIANGLES );
                        SPRT_16& pPoly = _orderTable->sp16;

//					float u0 = pPoly.u0;
//					float v0 = pPoly.v0;
//					u0 /= VRAM_WIDTHF;
//					v0 /= VRAM_HEIGHTF;
//					glTexCoord2f( u0, v0 );
                        glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
#if defined( GFX_ZBUFFER )
// nlin: uh, what does this do?	need to change to use 3d coords for persp. correction				glVertex2i( pPoly.x0, - (pPoly.y0) );
#else /* GFX_ZBUFFER */
                        glVertex2i( pPoly.x0, -pPoly.y0 );
#endif /* GFX_ZBUFFER */

//					float u1 = pPoly.u0 + 16;
//					float v1 = pPoly.v0 + 16;
//					u1 /= VRAM_WIDTHF;
//					v1 /= VRAM_HEIGHTF;
//					glTexCoord2f( u1, v1 );
                        glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
#if defined ( GFX_ZBUFFER )

// nlin: uh, what does this do?need to change to use 3d coords for persp. correction					glVertex2i( pPoly.x0 + 16, - (pPoly.y0 + 16) );
#else /* GFX_ZBUFFER */
                        glVertex2i( pPoly.x0 + 16,  -pPoly.y0 + 16 );
#endif /* GFX_ZBUFFER */
                        glEnd();
                        AssertGLOK();
                        glEnable( GL_TEXTURE_2D );
                        AssertGLOK();
                        break;
                    }

                default:
                    {
                        DBSTREAM1( std::cout << "code = " << int( code ) << std::endl; )
                        break;
                    }
            }
        }
    }
    AssertGLOK();
    glDisable(GL_TEXTURE_2D);
    AssertGLOK();
}

//==============================================================================

#endif									// defined(USE_ORDER_TABLES)


//==============================================================================

void
LoadGLMatrixFromMatrix34(const Matrix34& matrix)
{

    GLfloat mat[16];
    mat[(0*4)+0] = matrix[0][0].AsFloat();
    mat[(0*4)+1] = matrix[0][1].AsFloat();
    mat[(0*4)+2] = matrix[0][2].AsFloat();
    mat[(0*4)+3] = 0;

    mat[(1*4)+0] = matrix[1][0].AsFloat();
    mat[(1*4)+1] = matrix[1][1].AsFloat();
    mat[(1*4)+2] = matrix[1][2].AsFloat();
    mat[(1*4)+3] = 0;

    mat[(2*4)+0] = matrix[2][0].AsFloat();
    mat[(2*4)+1] = matrix[2][1].AsFloat();
    mat[(2*4)+2] = matrix[2][2].AsFloat();
    mat[(2*4)+3] = 0;

    mat[(3*4)+0] = matrix[3][0].AsFloat();
    mat[(3*4)+1] = matrix[3][1].AsFloat();
    mat[(3*4)+2] = matrix[3][2].AsFloat();
    mat[(3*4)+3] = 1.0;

    glLoadMatrixf(mat);
}

//==============================================================================


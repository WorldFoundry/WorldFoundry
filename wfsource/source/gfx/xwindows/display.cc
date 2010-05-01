//=============================================================================
// gfx/win/display.cc: display hardware abstraction class, windows openGL specific code
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
//       hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#define SIXTEEN_BIT_VRAM 0
// 0 = 32 bit, seperated RGB, 1 = packed 5 bits per pixel

#include <hal/hal.h>
#include <GL/gl.h>
#include <memory/memory.hp>
#include <gfx/pixelmap.hp>
extern bool bFullScreen;
extern int _halWindowWidth;
extern int _halWindowHeight;


//#       include <gl/glaux.h>
#       include <gfx/xwindows/wfprim.h>

#include <math.h>

// Keep track of windows changing width and height
GLfloat windowXPos;
GLfloat windowYPos;
GLfloat windowWidth;
GLfloat windowHeight;

#if defined(__WIN__)
#include "wgl.cc"
#endif
#if defined(__LINUX__)
#include "mesa.cc"
#include <sys/time.h>
#include <unistd.h>
#endif

//==============================================================================

Display::Display(int orderTableSize, int xPos, int yPos, int xSize, int ySize, Memory& memory,bool interlace) :
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
    if( !InitWindow(xPos, yPos, _halWindowWidth, _halWindowHeight ) )
    {
		printf("Display::Display:doInit Failed!\n");
		sys_exit(1);
    }
	assert(orderTableSize > 0);
#if defined(USE_ORDER_TABLES)
	for(int index=0;index<ORDER_TABLES;index++)
	{
		_orderTable[index] = new (_memory) OrderTable(orderTableSize,_memory);
		assert(ValidPtr(_orderTable[index]));
	}
#endif

	_drawPage = 0;
	ResetTime();

#if 0
#pragma message ("KTS " __FILE__ ": temp test code")
	for ( int y=0; y<VRAM_HEIGHT; ++y )
	{
		for ( int x=0; x<VRAM_WIDTH; ++x )
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

void
Display::ResetTime()					// used to reset delta timer for PageFlip
{
	//_clockLastTime = timeGetTime();  	//clock();

#if defined(__WIN__)
	_clockLastTime = timeGetTime();  				//clock();
#elif defined(__LINUX__)
    struct timeval tv;
    gettimeofday(&tv,NULL);
    _clockLastTime = (tv.tv_sec + tv.tv_usec*0.000001) * CLOCKS_PER_SEC;
#else
#error platform not supported
#endif
}

//============================================================================

void
Display::RenderBegin()
{
}

//==============================================================================

void
Display::RenderEnd()
{
}

//==============================================================================

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

      switch (event.type) 
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


	Validate();
	AssertMsg( _drawPage == 0 || _drawPage == 1, "_drawPage = " << _drawPage );
	glClearColor( _backgroundColorRed, _backgroundColorGreen, _backgroundColorBlue, 1.0 );
    AssertGLOK();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear the window with current clearing color
    AssertGLOK();

#if defined(__WIN__)
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) == TRUE)
	{
        if (GetMessage(&msg, NULL, 0, 0))
		{
            TranslateMessage(&msg);
			DispatchMessage(&msg);
        }
		else
		{
			finiObjects();
            sys_exit(1);
		}
	}
#endif
#if defined(__LINUX__)
    XEventLoop(); 
#endif
//FntPrint("\nWorld Foundry Display: page %d\n",_drawPage);
		glClearColor( _backgroundColorRed, _backgroundColorGreen, _backgroundColorBlue, 1.0 );
    AssertGLOK();
	//	glClear( GL_COLOR_BUFFER_BIT );
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear the window with current clearing color
    AssertGLOK();
		glDisable(GL_LIGHTING);
    AssertGLOK();

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
#else
#error now what?
		assert(0);
#endif
#else
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
#endif

		glFlush();
    AssertGLOK();

#if defined(__WIN__)
		// Call function to swap the buffers
		SwapBuffers(hardwaredevicecontext);

		// Validate the newly painted client area
		ValidateRect(worldFoundryhWnd,NULL);
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
	unsigned long now = timeGetTime();  				//clock();
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
    unsigned long now = (tv.tv_sec + tv.tv_usec*0.000001)*CLOCKS_PER_SEC;
    unsigned long delta = now - _clockLastTime;
    _clockLastTime = now;

	AssertMsg(delta/CLOCKS_PER_SEC < 65536, "delta/CLOCKS_PER_SEC = " << delta/CLOCKS_PER_SEC);
	Scalar whole(short(delta/CLOCKS_PER_SEC),0);
	Scalar frac(short(delta%CLOCKS_PER_SEC),0);
	frac /= SCALAR_CONSTANT(CLOCKS_PER_SEC);
	return(whole+frac);
#else
#error platform not defined
#endif
    
}

//============================================================================

#if defined(USE_ORDER_TABLES)

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
#error not yet!
    texturePixelMap.SetGLTexture();

    float uResult(??);                            
    float vResult(??);                           

#endif
    glTexCoord2f(uResult, vResult);                         
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


#define YSIZE 240

// #define checkImageWidth 64
// #define checkImageHeight 64
// extern GLubyte checkImage[checkImageHeight][checkImageWidth][4];

	glEnable( GL_TEXTURE_2D );
    AssertGLOK();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    AssertGLOK();

	for ( ; !isendprim( _orderTable ); _orderTable = (Primitive*)nextPrim( _orderTable ) )
	{
		ValidatePtr(_orderTable);
		Primitive* pTag = _orderTable;
		uint8 code = pTag->base.code;
#if 0
		std::cout << "otable code = " << int(_orderTable->code) << std::endl;
		std::cout << "code = " << int(code) << std::endl;
#endif
		if ( code )
		{
			switch ( code )
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
					glVertex2i( pPoly.x0, YSIZE-pPoly.y0 );
					glVertex2i( pPoly.x1, YSIZE-pPoly.y1 );
					glVertex2i( pPoly.x2, YSIZE-pPoly.y2 );
					glEnd();
    AssertGLOK();
					glEnable( GL_TEXTURE_2D );
    AssertGLOK();
					break;
				}

				case CODE_POLY_FT3:
				{
//					assert(0);
 					glBegin( GL_TRIANGLES );
					POLY_FT3& pPoly = _orderTable->ft3;

					//glColor3ub( rand() % 255, rand() % 255, rand() % 255 );
					//assert( theTexture );
					//glCallList( theTexture );
					glColor3ub( 255, 0, 0 );
                    assert(pPoly.pPixelMap);
                    CalcAndSetUV(pPoly.tpage, pPoly.u0,pPoly.v0,*pPoly.pPixelMap);
					glVertex2i( pPoly.x0, YSIZE-pPoly.y0 );
                    CalcAndSetUV(pPoly.tpage, pPoly.u1,pPoly.v1,*pPoly.pPixelMap);
					glVertex2i( pPoly.x1, YSIZE-pPoly.y1 );
                    CalcAndSetUV(pPoly.tpage, pPoly.u2,pPoly.v2,*pPoly.pPixelMap);
					glVertex2i( pPoly.x2, YSIZE-pPoly.y2 );
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

					glColor3ub( pPoly.r0, pPoly.g0, pPoly.b0 );
					glVertex2i( pPoly.x0, YSIZE-pPoly.y0 );
					glColor3ub( pPoly.r1, pPoly.g1, pPoly.b1 );
					glVertex2i( pPoly.x1, YSIZE-pPoly.y1 );
					glColor3ub( pPoly.r2, pPoly.g2, pPoly.b2 );
					glVertex2i( pPoly.x2, YSIZE-pPoly.y2 );
					glEnd();
    AssertGLOK();
					glEnable( GL_TEXTURE_2D );
    AssertGLOK();
					break;
				}

				case CODE_POLY_GT3:
				{

 					glBegin( GL_TRIANGLES );
					POLY_GT3& pPoly = _orderTable->gt3;

                    assert(pPoly.pPixelMap);
                    CalcAndSetUV(pPoly.tpage, pPoly.u0,pPoly.v0,*pPoly.pPixelMap);
					glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
					glVertex2i( pPoly.x0, YSIZE-pPoly.y0 );

                    CalcAndSetUV(pPoly.tpage, pPoly.u1,pPoly.v1,*pPoly.pPixelMap);

					glColor3f( float(pPoly.r1)/128, float(pPoly.g1)/128, float(pPoly.b1)/128 );
					glVertex2i( pPoly.x1, YSIZE-pPoly.y1 );

                    CalcAndSetUV(pPoly.tpage, pPoly.u2,pPoly.v2,*pPoly.pPixelMap);
					glColor3f( float(pPoly.r2)/128, float(pPoly.g2)/128, float(pPoly.b2)/128 );
					glVertex2i( pPoly.x2, YSIZE-pPoly.y2 );
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
					glVertex2i( pPoly.x0, YSIZE - pPoly.y0 );

//					float u1 = pPoly.u0 + 16;
//					float v1 = pPoly.v0 + 16;
//					u1 /= VRAM_WIDTHF;
//					v1 /= VRAM_HEIGHTF;
//					glTexCoord2f( u1, v1 );
					glColor3f( float(pPoly.r0)/128, float(pPoly.g0)/128, float(pPoly.b0)/128 );
					glVertex2i( pPoly.x0 + 16, YSIZE - pPoly.y0 + 16 );
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


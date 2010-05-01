//=============================================================================
// gfx/win/wgl.cc: windows gl specific portion of interface, included by display.cc
// Copyright ( c ) 1999 World Foundry Group  
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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <time.h>

#include <ddraw.h>


HWND glHwnd;
HWND	worldFoundryhWnd;		// Storeage for window handle
BOOL                    bActive;        // is application active?
static LPCTSTR lpszAppName = "World Foundry® Game Engine";

// Declaration for Window procedure
LRESULT CALLBACK WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

LPDIRECTDRAW            lpDD;           // DirectDraw object
#pragma comment( lib, "ddraw.lib" )
HPALETTE hPalette = NULL;
static HDC hardwaredevicecontext=0;			// Private GDI Device context
static HGLRC hRC=0;		// Permenant Rendering context

// Set Pixel Format function - forward declaration
void SetDCPixelFormat(HDC);


//==============================================================================
// finiObjects: finished with all objects we use; release them
static void
finiObjects( void )
{
	// Deselect the current rendering context and delete it
	if(!wglMakeCurrent(hardwaredevicecontext,NULL))
	{
		assert(0);
	}
	wglDeleteContext(hRC);

	// Delete the palette
	if(hPalette != NULL)
		DeleteObject(hPalette);

    if( lpDD != NULL )
    {
		lpDD->Release();
		lpDD = NULL;
    }
}

//==============================================================================
//=============================================================================

void
ChangeSize(GLsizei w, GLsizei h)
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Reset the coordinate system before modifying
    glLoadIdentity();


	// Keep the square square, this time, save calculated
	// width and height for later use
	if (w <= h)
	{
		windowHeight = 250.0f*h/w;
		windowWidth = 250.0f;
	}
    else
	{
		windowWidth = 250.0f*w/h;
		windowHeight = 250.0f;
	}

	// Set the clipping volume
	glOrtho(0.0f, windowWidth, 0.0f, windowHeight, 1.0f, -1.0f);
}

//==============================================================================

char szMsg[] = "Page Flipping Test: Press F12 to exit";
char szFrontMsg[] = "Front buffer (F12 to quit)";
char szBackMsg[] = "Back buffer (F12 to quit)";


// InitWindow - do work required for every instance of the application:
//                create the window, initialize data
static BOOL
InitWindow( int xPos, int yPos, int xSize, int ySize )
{
	WNDCLASS	wc;			// Windows class structure
    HRESULT             ddrval;

    // set up and register window class
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName = lpszAppName;
    wc.lpszClassName = lpszAppName;
	// Register the window class
	if(RegisterClass(&wc) == 0)
	{
		assert(0);
		sys_exit(1);
	}

    glHwnd = worldFoundryhWnd = CreateWindowEx(			// create a window
	0,
	lpszAppName,
	lpszAppName,
	( bFullScreen ? WS_POPUP : WS_OVERLAPPEDWINDOW ) /*| WS_CLIPCHILDREN | WS_CLIPSIBLINGS*/,
	// Window position and size
	xPos, yPos,
	xSize, ySize,
//	100, 100,
//	640, 480,
	NULL,
	NULL,
	NULL,
	NULL );
	ChangeSize( xSize, ySize );

	// If window was not created, quit
	if(worldFoundryhWnd == NULL)
	{
		assert(0);
		return FALSE;
	}

	// Display the window
	ShowWindow(worldFoundryhWnd,SW_SHOW);
	UpdateWindow(worldFoundryhWnd);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	if ( bFullScreen )
	{
    	ddrval = DirectDrawCreate( NULL, &lpDD, NULL );
    	assert( ddrval == DD_OK );

		ddrval = lpDD->SetCooperativeLevel( worldFoundryhWnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT );
		assert(ddrval == DD_OK );

   		ddrval = lpDD->SetDisplayMode( _halWindowWidth, _halWindowHeight, 16 );
		if ( ddrval != DD_OK )
		{
			char msg[ 128 ];
			sprintf( msg, "Selected video resolution (%d x %d) not supported by your video card", _halWindowWidth, _halWindowHeight );
			FatalError( msg );
		}
	}
	return TRUE;
}
//=============================================================================

// If necessary, creates a 3-3-2 palette for the device context listed.
HPALETTE GetOpenGLPalette(HDC hdc)
{
	HPALETTE hRetPal = NULL;	// Handle to palette to be created
	PIXELFORMATDESCRIPTOR pfd;	// Pixel Format Descriptor
	LOGPALETTE *pPal;			// Pointer to memory for logical palette
	int nPixelFormat;			// Pixel format index
	int nColors;				// Number of entries in palette
	int i;						// Counting variable
	BYTE RedRange,GreenRange,BlueRange;
								// Range for each color entry (7,7,and 3)

	// Get the pixel format index and retrieve the pixel format description
	nPixelFormat = GetPixelFormat(hdc);
	DescribePixelFormat(hdc, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	// Does this pixel format require a palette?  If not, do not create a
	// palette and just return NULL
	if(!(pfd.dwFlags & PFD_NEED_PALETTE))
		return NULL;

	// Number of entries in palette.  8 bits yeilds 256 entries
	nColors = 1 << pfd.cColorBits;

	// Allocate space for a logical palette structure plus all the palette entries
	pPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +nColors*sizeof(PALETTEENTRY));

	// Fill in palette header
	pPal->palVersion = 0x300;		// Windows 3.0
	pPal->palNumEntries = nColors; // table size

	// Build mask of all 1's.  This creates a number represented by having
	// the low order x bits set, where x = pfd.cRedBits, pfd.cGreenBits, and
	// pfd.cBlueBits.
	RedRange = (1 << pfd.cRedBits) -1;
	GreenRange = (1 << pfd.cGreenBits) - 1;
	BlueRange = (1 << pfd.cBlueBits) -1;

	// Loop through all the palette entries
	for(i = 0; i < nColors; i++)
	{
		// Fill in the 8-bit equivalents for each component
		pPal->palPalEntry[i].peRed = (i >> pfd.cRedShift) & RedRange;
		pPal->palPalEntry[i].peRed = (unsigned char)(
			(double) pPal->palPalEntry[i].peRed * 255.0 / RedRange);

		pPal->palPalEntry[i].peGreen = (i >> pfd.cGreenShift) & GreenRange;
		pPal->palPalEntry[i].peGreen = (unsigned char)(
			(double)pPal->palPalEntry[i].peGreen * 255.0 / GreenRange);

		pPal->palPalEntry[i].peBlue = (i >> pfd.cBlueShift) & BlueRange;
		pPal->palPalEntry[i].peBlue = (unsigned char)(
			(double)pPal->palPalEntry[i].peBlue * 255.0 / BlueRange);

		pPal->palPalEntry[i].peFlags = (unsigned char) NULL;
	}


	// Create the palette
	hRetPal = CreatePalette(pPal);

	// Go ahead and select and realize the palette for this device context
	SelectPalette(hdc,hRetPal,FALSE);
	RealizePalette(hdc);

	// Free the memory used for the logical palette structure
	free(pPal);

	// Return the handle to the new palette
	return hRetPal;
}

//=============================================================================
// Select the pixel format for a given device context

void SetDCPixelFormat(HDC hdc)
{
	int nPixelFormat;

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Size of this structure
		1,								// Version of this structure
		PFD_DRAW_TO_WINDOW |			// Draw to Window (not to bitmap)
		PFD_SUPPORT_OPENGL |			// Support OpenGL calls in window
		PFD_DOUBLEBUFFER|
		0,				// Double buffered mode
		PFD_TYPE_RGBA,					// RGBA Color mode
		16,								// Want 16 bit color
		0,0,0,0,0,0,					// Not used to select mode
		0,0,							// Not used to select mode
		0,0,0,0,0,						// Not used to select mode
		16,								// Size of depth buffer
		0,								// Not used to select mode
		0,								// Not used to select mode
		PFD_MAIN_PLANE,					// Draw in main plane
		0,								// Not used to select mode
		0,0,0							// Not used to select mode
	};
	// Choose a pixel format that best matches that described in pfd
	nPixelFormat = ChoosePixelFormat(hdc, &pfd);

	// Set the pixel format for the device context
	SetPixelFormat(hdc, nPixelFormat, &pfd);
}

//============================================================================

LRESULT CALLBACK WindowProc(	HWND 	hWnd,
							UINT	message,
							WPARAM	wParam,
							LPARAM	lParam)
{

    PAINTSTRUCT ps;
    RECT        rc;
    SIZE        size;
    static BYTE phase = 0;
	static count=0;

    switch( message )
    {
    	case WM_ACTIVATEAPP:
			bActive = wParam;
			break;

		case WM_CREATE:			// Window creation, setup for OpenGL
			// Store the device context
			hardwaredevicecontext = GetDC(hWnd);
			assert(hardwaredevicecontext);
			// Select the pixel format
			SetDCPixelFormat(hardwaredevicecontext);

			// Create the rendering context and make it current
			assert(hardwaredevicecontext);
			hRC = wglCreateContext(hardwaredevicecontext);
			assert(hRC);
			if(!wglMakeCurrent(hardwaredevicecontext, hRC))
				assert(0);

			// Create the palette
			hPalette = GetOpenGLPalette(hardwaredevicecontext);

			// Create a timer that fires every millisecond
			SetTimer(hWnd,101,1,NULL);
			break;

		case WM_DESTROY:			// Window is being destroyed, cleanup
			finiObjects();
			// Tell the application to terminate after the window is gone.
			PostQuitMessage(0);
			break;

		case WM_SIZE:			// Window is resized.
			// Call our function which modifies the clipping
			// volume and viewport
			ChangeSize(LOWORD(lParam), HIWORD(lParam));
			break;
    	case WM_TIMER:
		break;

    	case WM_KEYDOWN:
		switch( wParam )
		{
			case VK_ESCAPE:
//			case VK_F12:
	    		PostMessage(hWnd, WM_CLOSE, 0, 0);
	    		break;
        	default:   // Passes it on if unproccessed
            	return (DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;

		case WM_PAINT:				// The painting function.  This message sent by Windows whenever the screen needs updating.
		{
			// Call OpenGL drawing code
			glClearColor(0.0f, 0.0f, 1.0f, 1.0f);		// Set background clearing color to blue
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear the window with current clearing color
//			RenderScene();
			glFlush();

			// Call function to swap the buffers
			//SwapBuffers(hardwaredevicecontext);

			// Validate the newly painted client area
			ValidateRect(hWnd,NULL);
		}
		break;

		// Windows is telling the application that it may modify
		// the system palette.  This message in essance asks the
		// application for a new palette.
		case WM_QUERYNEWPALETTE:
			// If the palette was created.
			if(hPalette)
			{
				int nRet;

				// Selects the palette into the current device context
				SelectPalette(hardwaredevicecontext, hPalette, FALSE);

				// Map entries from the currently selected palette to
				// the system palette.  The return value is the number
				// of palette entries modified.
				nRet = RealizePalette(hardwaredevicecontext);

				// Repaint, forces remap of palette in current window
				InvalidateRect(hWnd,NULL,FALSE);

				return nRet;
			}
			break;

		// This window may set the palette, even though it is not the
		// currently active window.
		case WM_PALETTECHANGED:
			// Don't do anything if the palette does not exist, or if
			// this is the window that changed the palette.
			if((hPalette != NULL) && ((HWND)wParam != hWnd))
			{
				// Select the palette into the device context
				SelectPalette(hardwaredevicecontext,hPalette,FALSE);

				// Map entries to system palette
				RealizePalette(hardwaredevicecontext);

				// Remap the current colors to the newly realized palette
				UpdateColors(hardwaredevicecontext);
				return 0;
			}
			break;

    	case WM_SETCURSOR:				// turn mouse pointer off
		{
			if ( bFullScreen )
				SetCursor( NULL );
			return TRUE;
		}

        default:   // Passes it on if unproccessed
            return (DefWindowProc(hWnd, message, wParam, lParam));

    }
    //return DefWindowProc(hWnd, message, wParam, lParam);
	return(0L);
} // WindowProc


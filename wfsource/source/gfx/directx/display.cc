//=============================================================================
// gfx/directx/display.cc: display hardware abstraction class, windows directx specific code
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

#include <pigsys/pigsys.hp>
#include <hal/hal.h>
#include <memory/memory.hp>
#include <gfx/pixelmap.hp>
#include <gfx/directx/display.hp>
#include <gfx/directx/common.hp>
#include <gfx/directx/winmain.hp>
#include <gfx/directx/wfprim.h>
#include <gfx/directx/winproc.hp>
#include <gfx/directx/d3dtex.hp>
#include <gfx/directx/getdxver.hp>
#include <gfx/directx/scene.hp>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <cstdlib>
#include <cstdio>
//#include <cstdarg>
#include <time.h>
#define D3D_OVERLOADS
#include <ddraw.h>
#include <d3d.h>
#include <math.h>

//==============================================================================

HWND	worldFoundryhWnd;		// Storeage for window handle
//BOOL    bActive;        // is application active?

char szMsg[] = "Page Flipping Test: Press F12 to exit";

extern bool bFullScreen;
extern int _halWindowWidth;
extern int _halWindowHeight;

// #pragma pack( 1 )
// struct RGB_pixel
// {
//     unsigned char r, g, b;
// };
// #pragma pack( 4 )

//HPALETTE hPalette = NULL;
//static HDC hardwaredevicecontext=0;			// Private GDI Device context


//==============================================================================

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
//			bActive = wParam;
			break;

		case WM_CREATE:			// Window creation, setup for DirectX
			if(!bFullScreen)
				SetTimer(hWnd,101,1,NULL);		// Create a timer that fires every millisecond
			break;
		case WM_DESTROY:			// Window is being destroyed, cleanup

    		FiniMain ();
			//D3DRelease();
			// Tell the application to terminate after the window is gone.
			PostQuitMessage(0);
			break;
		case WM_SIZE:			// Window is resized.
			// Call our function which modifies the clipping
			// volume and viewport
//			ChangeSize(LOWORD(lParam), HIWORD(lParam));
			break;
    	case WM_TIMER:
		break;

    	case WM_KEYDOWN:
		switch( wParam )
		{
			case VK_ESCAPE:
			case VK_F12:
	    		PostMessage(hWnd, WM_CLOSE, 0, 0);
	    		break;
        	default:   // Passes it on if unproccessed
            	return (DefWindowProc(hWnd, message, wParam, lParam));
		}
		break;

		case WM_PAINT:				// The painting function.  This message sent by Windows whenever the screen needs updating.
		// Validate the newly painted client area
		if(bFullScreen)
			ValidateRect(hWnd,NULL);
		else
		{
			count++;
			BeginPaint( hWnd, &ps );
			GetClientRect(hWnd, &rc);
			GetTextExtentPoint( ps.hdc, szMsg, lstrlen(szMsg), &size );
			SetBkColor( ps.hdc, RGB( 0, 0, 255 ) );
			SetTextColor( ps.hdc, RGB( 255, 255, 0 ) );
			TextOut( ps.hdc, (rc.right - size.cx)/2, (rc.bottom - size.cy)/2,
	    		szMsg, sizeof( szMsg )-1 );
			char buffer[100];
			sprintf(buffer,"count = %\n",count);
			TextOut( ps.hdc, (rc.right - size.cx)/2, (rc.bottom - size.cy)/2,
	    		buffer, strlen(buffer) );

			EndPaint( hWnd, &ps );
		}
		break;

#if 0
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
#endif
    	case WM_SETCURSOR:				// turn mouse pointer off
			if ( bFullScreen )
				SetCursor(NULL);
			return TRUE;
        default:   // Passes it on if unproccessed
            return (DefWindowProc(hWnd, message, wParam, lParam));

    }
    //return DefWindowProc(hWnd, message, wParam, lParam);
	return(0L);
} // WindowProc

//==============================================================================


extern int InitMain ();
extern void FiniMain ();
extern BOOL InitMainWindow (void);

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

    DWORD dwDxVersion;
    DWORD dwDxPlatform;

    // Get hInstance handle
    g_hMainInstance = (HINSTANCE) GetModuleHandle (NULL);
    if (NULL == g_hMainInstance)
		FatalError("InitApp - GetModuleHandle() failed.");

    // Check for DX5.0 or greater
    GetDXVersion (&dwDxVersion, &dwDxPlatform);
    if (dwDxVersion < 0x500)
        FatalError("This App requires DX 5.0 or greater in order to run.");

    // Check for Previous instance
    if (! CheckPreviousApp ())
    {
        FatalError("previousApp");
    }

    // Init main window
    if (! InitMainWindow ())
		FatalError(__FILE__ ": InitMainWindow");

	worldFoundryhWnd = g_hMainWindow;
    printf("InitApp - Success");

#if 0
	_memory.Validate();

	DWORD dxVersion;
	DWORD dxPlatform;
	GetDXVersion(&dxVersion, &dxPlatform);

	if(dxVersion < 0x500)
		FatalError("DirectX5 Required\n");

    if( !InitDirectDrawDisplay(xPos, yPos, _halWindowWidth, _halWindowHeight ) )
    {
		printf("Display::Display:InitDirectDrawDisplay Failed!\n");
		exit(1);
    }

#if 0
	// ok, now create lights
	for(int lightIndex=0;lightIndex<LIGHT_COUNT;lightIndex++)
	{
    	HRESULT hResult = lpD3D->CreateLight(&lpd3dLight[lightIndex], NULL);
		DDCheckError(hResult,"CreateLight failed");
		assert(ValidPtr(lpd3dLight[lightIndex]));

    	ZeroMemory(&d3dLight[lightIndex], sizeof(d3dLight[lightIndex]));
    	d3dLight[lightIndex].dwSize = sizeof(d3dLight[lightIndex]);

    	d3dLight[lightIndex].dltType = D3DLIGHT_DIRECTIONAL;
    	d3dLight[lightIndex].dcvColor.dvR    = D3DVAL( 1.0);
    	d3dLight[lightIndex].dcvColor.dvG    = D3DVAL( 1.0);
    	d3dLight[lightIndex].dcvColor.dvB    = D3DVAL( 1.0);
    	d3dLight[lightIndex].dcvColor.dvA    = D3DVAL( 1.0);
    	d3dLight[lightIndex].dvPosition.dvX  = D3DVAL( 1.0);
    	d3dLight[lightIndex].dvPosition.dvY  = D3DVAL(-1.0);
    	d3dLight[lightIndex].dvPosition.dvZ  = D3DVAL(-1.0);
    	d3dLight[lightIndex].dvAttenuation0  = D3DVAL( 1.0);
    	d3dLight[lightIndex].dvAttenuation1  = D3DVAL( 0.0);
    	d3dLight[lightIndex].dvAttenuation2  = D3DVAL( 0.0);
    	d3dLight[lightIndex].dvRange         = D3DLIGHT_RANGE_MAX;
    	d3dLight[lightIndex].dvFalloff		 = 1.0f;
    	d3dLight[lightIndex].dwFlags		 = D3DLIGHT_ACTIVE;
    	d3dLight[lightIndex].dcvColor.r		 = 1.0f;
    	d3dLight[lightIndex].dcvColor.g		 = 1.0f;
    	d3dLight[lightIndex].dcvColor.b		 = 0.5f;
    	d3dLight[lightIndex].dvDirection.x		 = 1.0;
    	d3dLight[lightIndex].dvDirection.y		 = 0.0;
    	d3dLight[lightIndex].dvDirection.z		 = 0.0;

//    	hRes = lpd3dLight[lightIndex]->SetLight(&d3dLight[lightIndex]);
//		assert(hRes == DD_OK);
//    	if (hRes != DD_OK)
//			FatalError(__FILE__ ": cannot create lights");
	}

	assert(ValidPtr(lpd3dLight[0]));
    HRESULT hResult = lpd3dLight[0]->SetLight((D3DLIGHT*)&d3dLight[0]);
	DDCheckError(hResult,"SetLight failed");

    hResult = lpd3dViewport->AddLight(lpd3dLight[0]);
	DDCheckError(hResult,"AddLight failed");

    hResult = lpd3dDevice->SetLightState(D3DLIGHTSTATE_AMBIENT, RGBA_MAKE(100, 100, 100, 100));
	DDCheckError(hResult,"SetLightState failed");
#endif
#endif			                        // 0

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
	ASSERTIONS( _rendering = false; )

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
	assert(_rendering == false);
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
	Validate();
	_clockLastTime = timeGetTime();  	//clock();
}

//============================================================================

void
Display::RenderBegin()
{
	assert(_rendering == false);
	Validate();
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
			printf("Close message recieved, shutting down\n");
    		FiniMain ();
			//assert(0);
//			D3DRelease();
            exit(1);
		}
	}

    OnIdleBegin(g_hMainWindow);
    //OnIdle(g_hMainWindow);
#if defined(RENDERER_PIPELINE_DIRECTX)
//	lpd3dDevice->BeginScene();
#endif
	ASSERTIONS( _rendering = true );
}

//==============================================================================

void
Display::RenderEnd()
{
	Validate();
	assert(_rendering == true);

//	lpd3dDevice->EndScene();


    OnIdleEnd(g_hMainWindow);

	ASSERTIONS( _rendering = false );
}

//============================================================================
// used to fill surface with rainbow, so uninitialized texture space is noticable

void
FillSurface(LPDIRECTDRAWSURFACE ddSurface)
{
	DDSURFACEDESC ddsd;
	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	HRESULT hResult = ddSurface->Lock(NULL,&ddsd, 0, NULL);
	DDCheckError(hResult,"couldn't get surface lock");

	WORD* lpdMemory = (WORD*) ddsd.lpSurface;
	// kts check what the format of this surface is
	assert(ddsd.ddpfPixelFormat.dwFlags && DDPF_RGB);  // insure is rgb mode
	assert(ddsd.ddpfPixelFormat.dwRGBBitCount == SCREEN_BITS);  // insure 16 bits per pixel
	//printf("surface: %d, %d\n",ddsd.dwWidth, ddsd.dwHeight);

	for(int yPos = 0; yPos<ddsd.dwHeight; yPos++)
	{
		for(int xPos =0; xPos < ddsd.dwWidth; xPos++)
		{
			// kts pitch/2 since pitch is in bytes, and we are working in words
			lpdMemory[xPos+(yPos*(ddsd.lPitch/2))] = yPos*1000 + xPos*100;
		}
	}
	ddSurface->Unlock( NULL );
}


//============================================================================

#if SW_DBSTREAM


std::ostream&
operator<< (std::ostream& out, const DDSCAPS ddsCaps)
{
	out << "DDSCAPS: " << std::endl;
#define ENTRY(entry) \
	if(ddsCaps.dwCaps & entry) \
		out << "  " #entry << std::endl;

	ENTRY(DDSCAPS_RESERVED1);
	ENTRY(DDSCAPS_ALPHA);
	ENTRY(DDSCAPS_BACKBUFFER);
	ENTRY(DDSCAPS_COMPLEX);
	ENTRY(DDSCAPS_FLIP);
	ENTRY(DDSCAPS_FRONTBUFFER);
	ENTRY(DDSCAPS_OFFSCREENPLAIN);
	ENTRY(DDSCAPS_OVERLAY	);
	ENTRY(DDSCAPS_PALETTE);
	ENTRY(DDSCAPS_PRIMARYSURFACE);
	ENTRY(DDSCAPS_PRIMARYSURFACELEFT);
	ENTRY(DDSCAPS_SYSTEMMEMORY);
	ENTRY(DDSCAPS_TEXTURE);
	ENTRY(DDSCAPS_3DDEVICE);
	ENTRY(DDSCAPS_VIDEOMEMORY);
	ENTRY(DDSCAPS_VISIBLE);
	ENTRY(DDSCAPS_WRITEONLY);
	ENTRY(DDSCAPS_ZBUFFER);
	ENTRY(DDSCAPS_OWNDC);
	ENTRY(DDSCAPS_LIVEVIDEO);
	ENTRY(DDSCAPS_HWCODEC);
	ENTRY(DDSCAPS_MODEX);
	ENTRY(DDSCAPS_MIPMAP);
	ENTRY(DDSCAPS_RESERVED2);
	ENTRY(DDSCAPS_ALLOCONLOAD);
	ENTRY(DDSCAPS_VIDEOPORT);
	ENTRY(DDSCAPS_LOCALVIDMEM);
	ENTRY(DDSCAPS_NONLOCALVIDMEM);
	ENTRY(DDSCAPS_STANDARDVGAMODE);
	ENTRY(DDSCAPS_OPTIMIZED);
#undef ENTRY
	return out;
}

std::ostream&
operator<< (std::ostream& out, const DDPIXELFORMAT ddPixelFormat)
{
#define ENTRY(entry) \
	if(ddPixelFormat.dwFlags & entry) \
		out << "  " #entry << std::endl;

	out << "pixel format rgb count = " << ddPixelFormat.dwRGBBitCount << std::endl;
	out << "pixel format flags = " << ddPixelFormat.dwFlags << std::endl;
	ENTRY(DDPF_ALPHAPIXELS);
	ENTRY(DDPF_ALPHA);
	ENTRY(DDPF_FOURCC);
	ENTRY(DDPF_PALETTEINDEXED4);
	ENTRY(DDPF_PALETTEINDEXEDTO8);
	ENTRY(DDPF_PALETTEINDEXED8);
	ENTRY(DDPF_RGB);
	ENTRY(DDPF_COMPRESSED);
	ENTRY(DDPF_RGBTOYUV);
	ENTRY(DDPF_YUV);
	ENTRY(DDPF_ZBUFFER);
	ENTRY(DDPF_PALETTEINDEXED2);
	ENTRY(DDPF_ZPIXELS);
	out << "dwRBitMask = " << ddPixelFormat.dwRBitMask << std::endl;
	out << "dwGBitMask = " << ddPixelFormat.dwGBitMask << std::endl;
	out << "dwBBitMask = " << ddPixelFormat.dwBBitMask << std::endl;
	return out;
#undef ENTRY
}

std::ostream&
operator<< (std::ostream& out, const LPDIRECTDRAWSURFACE ddSurface)
{
	DDSURFACEDESC ddsd;
	memset(&ddsd,0,sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(ddsd);
	HRESULT hResult = ddSurface->Lock(NULL,&ddsd, 0, NULL);
	DDCheckError(hResult,"couldn't get surface lock");

	out << "DDSurface Dump:" << std::endl;
	out << "xysize: " << ddsd.dwWidth << "," << ddsd.dwHeight << std::endl;
	out << "pitch: " << ddsd.lPitch << std::endl;
	out << "backbuffercount: " << ddsd.dwBackBufferCount << std::endl;
	out << "zbufferbitdepth: " << ddsd.dwZBufferBitDepth << std::endl;
	out << "alphabitdepth: " << ddsd.dwAlphaBitDepth << std::endl;
	out << "lpsurface: " << ddsd.lpSurface << std::endl;

	out << ddsd.ddpfPixelFormat << std::endl;
	out << ddsd.ddsCaps;

	ddSurface->Unlock( NULL );
	return out;
}
#endif

//============================================================================

extern LPD3DWindow		g_lpd3dWin;

Scalar
Display::PageFlip()
{
	Validate();
	assert(_rendering == false);
	AssertMsg( _drawPage == 0 || _drawPage == 1, "_drawPage = " << _drawPage );


#if defined(RENDERER_PIPELINE_SOFTWARE)
#if defined(USE_ORDER_TABLES)
		lpd3dDevice->BeginScene();
		SetConstructionOrderTableIndex(_drawPage);
		SetRenderOrderTableIndex(1-_drawPage);
		_orderTable[GetRenderOrderTableIndex()]->Render();
		_drawPage ^= 1;
		_orderTable[GetConstructionOrderTableIndex()]->Clear();
		lpd3dDevice->EndScene();
#else
#error now what?
		assert(0);
#endif
#else
#if 0
// enable to see texture memory
	LPDIRECTDRAWSURFACE surface = videoMemoryTexture.GetVMemSurface();
//	FillSurface(surface);
//	cout << "backbuffer = " << g_lpd3dWin->GetBackBuffer() << std::endl;
	cout << "surface = " << surface << std::endl;

	// code to copy vmem to screen (so I could look at it)
	while(1)
	{
		RECT rect;
		rect.left = 320;
		rect.right = rect.left+320;
		rect.top = 0;
		rect.bottom = rect.top + 200;
		HRESULT hResult = g_lpd3dWin->GetBackBuffer()->BltFast( 0,0,
				surface, &rect, DDBLTFAST_NOCOLORKEY);

		if(hResult == DD_OK)
			break;
		if(hResult == DDERR_WASSTILLDRAWING)
			continue;
		DDCheckError(hResult, "BltFast failed\n");
	}
#endif
#endif

	// now calc how long it has been since last frame
	unsigned long now = timeGetTime();  				//clock();
	unsigned long delta = now - _clockLastTime;
	_clockLastTime = now;

	AssertMsg(delta/CLOCKS_PER_SEC < 65536, "delta/CLOCKS_PER_SEC = " << delta/CLOCKS_PER_SEC);
	Scalar whole(short(delta/CLOCKS_PER_SEC),0);
	Scalar frac(short(delta%CLOCKS_PER_SEC),0);
	frac /= SCALAR_CONSTANT(CLOCKS_PER_SEC);
	Scalar deltaTime = whole+frac;

#if 1
// enable to print frame rate
    HDC         hdc;
    static BYTE phase = 0;

	if (g_lpd3dWin->GetBackBuffer()->GetDC(&hdc) == DD_OK)
	{
		// Validate the newly painted client area
		ValidateRect(worldFoundryhWnd,NULL);

		SetBkColor( hdc, RGB( 0, 0, 255 ) );
		SetTextColor( hdc, RGB( 255, 255, 0 ) );

		char szMessage[500];
		sprintf(szMessage, "Page %d, frame rate: %f, %f (Alt-F4 to exit)",
			phase,
			1.0/(float((deltaTime).AsLong())/65536.0),
			float(deltaTime.AsLong())/65536.0
			);
		TextOut( hdc, 0, 0, szMessage, lstrlen(szMessage) );

		phase ^= 1;
		g_lpd3dWin->GetBackBuffer()->ReleaseDC(hdc);
	}
	else
		assert(0);
#endif


    OnIdlePageFlip(g_hMainWindow);



	return(deltaTime);
}

//==============================================================================

#if defined(USE_ORDER_TABLES)

extern LPDIRECT3DDEVICE2 lpd3dDevice;

void
DrawOTag( ORDER_TABLE_ENTRY* __orderTable )
{
	D3DTLVERTEX v[3];
	Primitive* _orderTable = (Primitive*)__orderTable;

	assert( _orderTable );
	Primitive* _orderTableEnd = _orderTable;

	assert( CODE_NOP == 0 );

	for ( ; !isendprim( _orderTable ); _orderTable = (Primitive*)nextPrim( _orderTable ) )
	{
		ValidatePtr(_orderTable);
		Primitive* pTag = _orderTable;
		uint8 code = pTag->code;
//		cout << "otable code = " << int(_orderTable->code) << std::endl;
//		cout << "code = " << int(code) << std::endl;
		if ( code )
		{
			switch ( code )
			{
				case CODE_POLY_F3:
				{
				// Set the current texture
				lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
					POLY_F3& pPoly = _orderTable->f3;

//#define BRIGHTNESS 256

#define CONVERTRGB(r,g,b) ((r<<16)|(g<<8)|b)
//#define CONVERTRGB(r,g,b) ( ((r<<17)&0xff0000)|((g<<9)&0xff00)| ((b<<1)&0xff) )

					v[0] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x0)*sceneXScale, float(pPoly.y0)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),0,0);
					v[1] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x1)*sceneXScale, float(pPoly.y1)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),0,0);
					v[2] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x2)*sceneXScale, float(pPoly.y2)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),0,0);
					lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
					// Set the current texture
					lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, d3dTextureHandle);
					break;
				}

				case CODE_POLY_FT3:
				{
// 					glBegin( GL_TRIANGLES );
					POLY_FT3& pPoly = _orderTable->ft3;

					v[0] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x0)*sceneXScale, float(pPoly.y0)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),0,0);
					v[1] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x1)*sceneXScale, float(pPoly.y1)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),0,1);
					v[2] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x2)*sceneXScale, float(pPoly.y2)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),1,1);
					lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
//					assert( theTexture );
//					glCallList( theTexture );
//					glColor3ub( 255, 0, 0 );
//					glTexCoord2f( 0.0, 0.0 );
//					glVertex2i( pPoly.x0, YSIZE-pPoly.y0 );
//					glTexCoord2f( 0.0, 1.0 );
//					glVertex2i( pPoly.x1, YSIZE-pPoly.y1 );
//					glTexCoord2f( 1.0, 1.0 );
//					glVertex2i( pPoly.x2, YSIZE-pPoly.y2 );
//					glEnd();
					break;
				}

				case CODE_POLY_G3:
				{
					// Set the current texture
					lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
					POLY_G3& pPoly = _orderTable->g3;
					v[0] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x0)*sceneXScale, float(pPoly.y0)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),0,0);
					v[1] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x1)*sceneXScale, float(pPoly.y1)*sceneYScale,0),1,CONVERTRGB(pPoly.r1,pPoly.g1,pPoly.b1),CONVERTRGB(0,0,0),0,0);
					v[2] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x2)*sceneXScale, float(pPoly.y2)*sceneYScale,0),1,CONVERTRGB(pPoly.r2,pPoly.g2,pPoly.b2),CONVERTRGB(0,0,0),0,0);
					lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
					// Set the current texture
					lpd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, d3dTextureHandle);
					break;
				}

				case CODE_POLY_GT3:
				{
					POLY_GT3& pPoly = _orderTable->gt3;
					float u0(pPoly.u0+DecodeTPageX(pPoly.tpage));
					float v0(pPoly.v0+DecodeTPageY(pPoly.tpage));
					u0 /= VRAM_WIDTHF;
					v0 /= VRAM_HEIGHTF;
					v[0] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x0)*sceneXScale, float(pPoly.y0)*sceneYScale,0),1,CONVERTRGB(pPoly.r0,pPoly.g0,pPoly.b0),CONVERTRGB(0,0,0),u0,v0);

					float u1(pPoly.u1+DecodeTPageX(pPoly.tpage));
					float v1(pPoly.v1+DecodeTPageY(pPoly.tpage));
					u1 /= VRAM_WIDTHF;
					v1 /= VRAM_HEIGHTF;

					v[1] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x1)*sceneXScale, float(pPoly.y1)*sceneYScale,0),1,CONVERTRGB(pPoly.r1,pPoly.g1,pPoly.b1),CONVERTRGB(0,0,0),u1,v1);

					float u2(pPoly.u2+DecodeTPageX(pPoly.tpage));
					float v2(pPoly.v2+DecodeTPageY(pPoly.tpage));
					u2 /= VRAM_WIDTHF;
					v2 /= VRAM_HEIGHTF;
					v[2] = D3DTLVERTEX(D3DVECTOR(float(pPoly.x2)*sceneXScale, float(pPoly.y2)*sceneYScale,0),1,CONVERTRGB(pPoly.r2,pPoly.g2,pPoly.b2),CONVERTRGB(0,0,0),u2,v2);
					lpd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST,D3DVT_TLVERTEX,(LPVOID)v,3,NULL);
					break;
				}
				default:
				{
					printf("hey, incorrect poly type\n");
					DBSTREAM1( cout << "code = " << int( code ) << std::endl; )
					break;
				}
			}
		}
	}
}

#endif						// defined(USE_ORDER_TABLES)

//==============================================================================

int32
LZC(uint32 value)
{
	int count = 0;
	while(count < 32)
	{
		if(value & 0x80000000)
			return count;
		value <<= 1;
		count++;
	}
	return count;
}


//==============================================================================

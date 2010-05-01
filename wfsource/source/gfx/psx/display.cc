//=============================================================================
// gfx\psx\display.cc: display hardware abstraction class, psx specific cod3e
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

#include <cstdio>

#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

#include <hal/hal.h>
#include <memory/memory.hp>
#include <gfx/pixelmap.hp>
#if defined(DO_SLOW_STEREOGRAM)
#	include <kernel.h>
#endif

#include <inline_c.h>
#include "..\rendmatt.hp"

//============================================================================

Display::Display(int orderTableSize, int xPos, int yPos, int xSize, int ySize,Memory& memory, bool interlace) :
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
	assert(xSize > 0);
	assert(xSize <= 640);
	assert(ySize > 0);
	assert(ySize <= 240);

	RangeCheck(0,orderTableSize, 10000);  // arbitrary, want to know if anyone needs a bigger one

#if defined(USE_ORDER_TABLES)
	for(int index=0;index<ORDER_TABLES;index++)
	{
		_orderTable[index] = new (_memory) OrderTable(orderTableSize,_memory);
		assert(ValidPtr(_orderTable[index]));
	}
#endif

	bg[0].x = 0;
	bg[0].y = 0;
	bg[0].w = xSize;
	bg[0].h = ySize;
	bg[1].x = 0;
	bg[1].y = ySize;			// shift down by size of bg[0]
	bg[1].w = xSize;
	bg[1].h = ySize;

	assert(xSize+xSize <= 1024);
	assert(ySize+ySize <= 512);			// make sure both pages fit into video memory

	// set up the gte, should we do this here?
	InitGeom();

	// set geometry origin at center of screen
	gte_SetGeomOffset(xSize/2, ySize/2);

	// distance to veiwing-screen:
//	gte_SetGeomScreen(512);
	gte_SetGeomScreen(110);

	ResetGraph(0);
	SetGraphDebug(0);
	FntLoad(960,256);
//	FntLoad(640,0);
	SetDumpFnt(FntOpen(0,12, _xSize, _ySize, 0, 1024));

	SetDefDrawEnv(&_drawEnv[0], bg[0].x, bg[0].y, bg[0].w, bg[0].h);
	SetDefDrawEnv(&_drawEnv[1], bg[1].x, bg[1].y, bg[1].w, bg[1].h);

	SetDefDispEnv(&_dispEnv[0], bg[0].x, bg[0].y, bg[0].w, bg[0].h);
	_dispEnv[0].isinter = interlace;
	SetDefDispEnv(&_dispEnv[1], bg[1].x, bg[1].y, bg[1].w, bg[1].h);
	_dispEnv[1].isinter = interlace;
	PutDrawEnv(&_drawEnv[_drawPage]);
	PutDispEnv(&_dispEnv[_drawPage]);
	SetDispMask(1);

	FntPrint("\nWorld Foundry Display\n");

	ClearImage(&bg[0],  _backgroundColor.Red(),  _backgroundColor.Green(), _backgroundColor.Blue());
	ClearImage(&bg[1],  _backgroundColor.Red(),  _backgroundColor.Green(), _backgroundColor.Blue());
//	ClearImage(&bg[0], 10, 50,10);
//	ClearImage(&bg[1], 10, 10,50);
	FntFlush(-1);
	DrawSync(0);
	VSync(0);
	SetDispMask(1);
	Validate();

	ResetTime();

	_videoMemory = new (HALLmalloc) PixelMap( PixelMap::MEMORY_VIDEO, VRAMWidth, VRAMHeight );
	assert( ValidPtr( _videoMemory ) );
}

//============================================================================

Display::~Display()
{
	Validate();
	if(_drawPage == 0)
		PageFlip();

	ClearImage(&bg[0], 0, 0, 0);
	PageFlip();

#if defined(USE_ORDER_TABLES)
	for(int index=0;index<ORDER_TABLES;index++)
	{
		MEMORY_DELETE(_memory,_orderTable[index],OrderTable);
	}
#endif
}

//============================================================================

void
Display::ResetTime()					// used to reset delta timer for PageFlip
{
	_resetClock = true;
}

//============================================================================

int
Display::VideoSync()
{
	Validate();
	int dSync = DrawSync(1);			// read # of otables left to execute
	if(dSync)
		FntPrint("RenderBound! %d\n",dSync);
	FntFlush(-1);
	FntPrint("\n World Foundry:page %d\n",_drawPage);

	DrawSync(0);						// wait for drawing to complete
//	while(DrawSync(1));
	int lines = VSync(0);
	return lines;
}

//==============================================================================

void
Display::UnSyncedPageFlip()
{
	Validate();
	_drawPage = !_drawPage;
	PutDispEnv(&_dispEnv[!_drawPage]);
	PutDrawEnv(&_drawEnv[_drawPage]);
}

//==============================================================================

#if defined(USE_ORDER_TABLES)

void
Display::ClearConstructionOrderTable()
{
	Validate();
	_orderTable[GetConstructionOrderTableIndex()]->Clear();
}

//==============================================================================

void
Display::RenderOrderTable()
{
	Validate();
	ClearImage(&bg[_drawPage],  _backgroundColor.Red(),  _backgroundColor.Green(), _backgroundColor.Blue());
	_orderTable[GetRenderOrderTableIndex()]->Render();
}

#endif		                // USE_ORDER_TABLES

//==============================================================================

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
	Validate();
	int lines = VideoSync();
	UnSyncedPageFlip();
#if defined(USE_ORDER_TABLES)
	SetConstructionOrderTableIndex(_drawPage);
	SetRenderOrderTableIndex(1-_drawPage);
	RenderOrderTable();
	ClearConstructionOrderTable();
#else
#error now what?
#endif
	int fields = lines/262;		// kts maybe different for pal
	Scalar deltaTime = Scalar::one / (SCALAR_CONSTANT(60) / Scalar(fields+1,0));
	if(_resetClock)
	{
		_resetClock = false;
		deltaTime = SCALAR_CONSTANT(0);
	}
	FntPrint(" fields %d:%d=%dfps\n",fields,lines%262,60/(fields+1));
	return(deltaTime);
}

//============================================================================

#if defined(DO_SLOW_STEREOGRAM)

volatile VSyncCallbackInterface vsyncCallbackInterface;

Scalar
VSyncCallBackReadVSyncCount()
{
//	cscreen << "fields = " << vsyncCallbackInterface.vsyncCount << endl;
	EnterCriticalSection();
	int fields = vsyncCallbackInterface.vsyncCount;
	vsyncCallbackInterface.vsyncCount = 0;
	ExitCriticalSection();

	if(vsyncCallbackInterface._display->_resetClock)
	{
		vsyncCallbackInterface._display->_resetClock = false;
		fields = 0;
	}

	Scalar deltaTime = Scalar::one / (SCALAR_CONSTANT(60) / Scalar(fields+1,0));
	return deltaTime;
}
//==============================================================================

void
VSyncCallbackFunction(void)
{
	static int phase=0;
	static int didRenderLastField = 0;

	if(DrawSync(0) != 0)
	{
		assert(0);
		FatalError("rendering overflowed 1/60th!\n");
		exit(1);
	}
	if(didRenderLastField)
	{
		FntFlush(-1);
		DrawSync(1);
		didRenderLastField = 0;
	}

	phase ^= 1;
	vsyncCallbackInterface.vsyncCount++;
	vsyncCallbackInterface.renderLock = -1;

	assert(ValidPtr(vsyncCallbackInterface._display));
	Display& display(*vsyncCallbackInterface._display);
	display.UnSyncedPageFlip();
	if(vsyncCallbackInterface.pagesDirty[phase])
	{
		display.SetRenderOrderTableIndex((phase)+vsyncCallbackInterface.orderTableOffset);
		display.RenderOrderTable();
		vsyncCallbackInterface.renderLock = (phase)+vsyncCallbackInterface.orderTableOffset;
		if(vsyncCallbackInterface.constructLock == vsyncCallbackInterface.renderLock)
		{
			FatalError(__FILE__ ": render lock failed\n");
			sys_exit(1);
		}
		vsyncCallbackInterface.pagesDirty[phase] = 0;
		didRenderLastField = 1;

		uint32 padd = PadRead(1);
		padd >>= 16;

		if(padd)
			FntPrint("\nWorld Foundry:page %d\n",display.GetDrawPageIndex());
	}
}

//==============================================================================

void
InitVSyncCallback(Display& display)
{
	vsyncCallbackInterface.orderTableOffset = 0;
	vsyncCallbackInterface.renderLock=-1;				// which order table is currently rendering
	vsyncCallbackInterface.constructLock=-1;			// which order table is currently being constructed

	for(int index=0;index < Display::DISPLAY_PAGES;index++)
		vsyncCallbackInterface.pagesDirty[index] = 0;
	vsyncCallbackInterface._display = &display;
	VSyncCallback( VSyncCallbackFunction );
}

//==============================================================================

void
UnInitVSyncCallback()
{
	VSync(0);
	VSync(0);
	VSyncCallback( 0 );
	RECT bg;
	bg.x = 0;
	bg.y = 0;
	bg.w = 320;
	bg.h = 480;
	ClearImage(&bg,  0,0,0);
}

#endif

//============================================================================

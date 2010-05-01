//============================================================================
// vmem.cc: video memory map managment
// Copyright (c) 1997,1998,1999,2000,2001 World Foundry Group.  
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

//============================================================================
// Original Author: Kevin T. Seghetti
//=============================================================================

#include <pigsys/pigsys.hp>
#include <hal/hal.h>
#include "vmem.hp"
#include "display.hp"

//=============================================================================

VideoMemory::VideoMemory(Display& display)
{
	for(int index=0;index < MAX_SLOTS;index++)
	{
		_textures[index] = NULL;
		_palettes[index] = NULL;
	}
	AllocateTextureSlots(display);
	Validate();
}

//=============================================================================

VideoMemory::~VideoMemory()
{
	Validate();
	for(int index=MAX_TRANSIENT_SLOTS-1;index >= 0;index--)
	{
		MEMORY_DELETE(HALLmalloc,_textures[index],PixelMap);
		MEMORY_DELETE(HALLmalloc,_palettes[index],PixelMap);
	}
	MEMORY_DELETE(HALLmalloc,_textures[PERMANENT_SLOT],PixelMap);
	MEMORY_DELETE(HALLmalloc,_palettes[PERMANENT_SLOT],PixelMap);
}

//=============================================================================

PixelMap*
VideoMemory::AllocateTextureSlot
(
	Display& /*display*/,
	const int16 basex, const int16 basey,
	const uint16 width, const uint16 height
)
{
#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)   
    PixelMap& videoMemory = display.GetVideoMemoryPixelMap();
	videoMemory.Validate();

	// is the rectangle in VRAM?
	assert( basex >= 0 && basex < Display::VRAMWidth );
	assert( basey >= 0 && basey < Display::VRAMHeight );
	assert( basex + width >= 0 && basex + width <= Display::VRAMWidth );
	assert( basey + height >= 0 && basey + height <= Display::VRAMHeight );

	PixelMap * slot = new (HALLmalloc) PixelMap( videoMemory, basex, basey, width, height );
#else
	PixelMap * slot = new (HALLmalloc) PixelMap( PixelMap::MEMORY_VIDEO, width, height );
#endif
	ValidatePtr( slot );
	return slot;
}

//=============================================================================

#if defined(DESIGNER_CHEATS)
static Color slotColors[VideoMemory::MAX_SLOTS] =
{
	Color(64,0,0),
	Color(0,64,0),
	Color(0,0,64),
	Color(32,32,32)
};

//-----------------------------------------------------------------------------

void
VideoMemory::ClearSlot(int slotIndex) const		// set slot back to background color, for debugging
{
	assert(slotIndex >= 0);
	assert(slotIndex < MAX_SLOTS);
	assert(ValidPtr(_textures[slotIndex]));
	_textures[slotIndex]->Clear(slotColors[slotIndex]);
	AssertMsg(ValidPtr(_palettes[slotIndex]),"slotIndex = " << slotIndex);
	_palettes[slotIndex]->Clear(slotColors[slotIndex]);
}

#endif
//=============================================================================

void
VideoMemory::AllocateTextureSlots(Display& display)
{
	// allocate the master palette area
	_palettes[PERMANENT_SLOT] = AllocateTextureSlot
	(
		display,
		VRAMPaletteBaseX,
		VRAMPaletteBaseY,
		VRAMPaletteWidth,
		VRAMPaletteHeight
	 );
	assert( ValidPtr( _palettes[PERMANENT_SLOT] ) );

	// allocate the permanent texture slot
	_textures[PERMANENT_SLOT] = AllocateTextureSlot
	(
		display,
		VRAMPermanentBaseX,
		VRAMPermanentBaseY,
		VRAMPermanentWidth,
		VRAMPermanentHeight
	 );
	assert( ValidPtr( _textures[PERMANENT_SLOT] ) );

	assert( MAX_TRANSIENT_SLOTS == 3 );
	// ^FAIL: may need to reconsider VRAM * constants w.r.t. PSX constraints

	// allocate the transient texture slots
	for( int transientIndex = 0; transientIndex < MAX_TRANSIENT_SLOTS; transientIndex++ )
	{
		_palettes[transientIndex] = AllocateTextureSlot
		(
		    display,
#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
			VRAMPaletteBaseX,
			uint16( VRAMPaletteBaseY + ( ( (int16)transientIndex + 1 ) * VRAMPaletteHeight ) ),
#else
            0,0,
#endif
			VRAMPaletteWidth,
			VRAMPaletteHeight
		 );
		assert( ValidPtr( _palettes[transientIndex] ) );

		_textures[transientIndex] = AllocateTextureSlot
		(
		    display,
#if defined(VIDEO_MEMORY_IN_ONE_PIXELMAP)
			uint16( VRAMTransientBaseX + ( (int16)transientIndex * VRAMTransientWidth ) ),
			VRAMTransientBaseY,
#else
            0,0,
#endif
			VRAMTransientWidth,
			VRAMTransientHeight
		 );
		assert( ValidPtr( _textures[transientIndex] ) );
	}

#if defined(DESIGNER_CHEATS)
    // so that designer can tell by looking at video memory which slots have been loaded
	for(int slotIndex=0;slotIndex<MAX_SLOTS;slotIndex++)
		ClearSlot(slotIndex);
#endif
}

//=============================================================================

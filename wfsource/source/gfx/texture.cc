//============================================================================
// texture.cc: texture map loaders
// Copyright (c) 1997,1999,2000,2001 World Foundry Group.  
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
#include <gfx/texture.hp>
#include <gfx/pixelmap.hp>
#include <iff/iffread.hp>
#include <streams/binstrm.hp>

#if defined (__PSX__)
#include <libgte.h>
#include <libgpu.h>
#endif

//=============================================================================

#pragma pack( 1 )
typedef struct
    {
#if 0
    char IDLength;
    char ColorMapType;
    char ImageType;
	int16 CMapStart;
	int16 CMapLength;
    char CMapDepth;
    int16 XOffset;
    int16 YOffset;
#else
	char foo[12];
#endif
    int16 Width;
    int16 Height;
    char PixelDepth;
    char ImageDescriptor;
    } TGA_HEADER;
#pragma pack()

//-----------------------------------------------------------------------------
// load tga from disk into video memory

void
LoadTexture(binistream& texturestream, PixelMap& map)
{
//	cout << "reading " << sizeof(TGA_HEADER) << " bytes " << endl;

	TGA_HEADER header;
	texturestream.read((char*)&header,sizeof(TGA_HEADER));
#pragma message ("KTS: add lots of tga validation here, for now assumes 16 bit uncompressed")
//	cout << "header.Width " << header.Width << ", header.Height " << header.Height << " bytes " << endl;
//	cout << "reading " << header.Width*header.Height*2 << " bytes " << endl;
	int width = header.Width;
	int height = header.Height;

	const void* buffer = texturestream.GetMemoryPtr(width * height * 2);
   RangeCheckExclusive(0,width,map.GetXSize()+1);
   RangeCheckExclusive(0,height,map.GetYSize()+1);
   assert(buffer);

		// kts hack, image needs to be long word aligned on disk
	int16* from = (int16*)buffer;
	int16* to = (int16*)((char*)buffer-2);
	for(int index=0;index < (width*height);index++)
	{
		*to++ = *from++;
	}

	buffer = (char*)buffer-2;
	assert(ValidPtr(buffer));
	map.Load(buffer,width, height);
//	HALLmalloc.Free(buffer,width*height*2);
}

//-----------------------------------------------------------------------------
// load tga from disk into video memory

void
LoadTexture(IFFChunkIter& textureIter, PixelMap& map)
{
   int size = textureIter.BytesLeft();
   const char* buffer = textureIter.GetMemoryPtr(size);
   binistream textureStream((void*)buffer,size);
   LoadTexture(textureStream,map);
}

//=============================================================================

//==============================================================================
// tga.cc
// by William B. Norris IV
// Copyright (c) 1995-1999, World Foundry Group  
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
//==============================================================================
// 08 Dec 95	WBNIV	Targa-specific instance of Bitmap class created
// 18 Jan 96	WBNIV	Swap color values around for Playstation version (RGB->BRG)
// 16 Feb 96	WBNIV   Prints enumeration list
// 07 Jul 96	WBNIV	Alpha channel support
// 10 Apr 97	WBNIV	Allows user to specify the transparent colour
//==============================================================================

#include "global.hp"
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <malloc.h>

using namespace std;

#include "textile.hp"
#include "types.h"
#include "bitmap.hp"
extern bool bVerbose;
extern bool bDebug;
extern TargetSystem ts;
extern bool bForceTranslucent;

//#include "memcheck.h"

TargaBitmap::~TargaBitmap()
{
}


TargaBitmap::TargaBitmap( const int width, const int height,
	const uint16 bgColor ) : Bitmap( width, height, bgColor )
{
}


bool
TargaBitmap::Validate( void* header, size_t size )
{
	if ( size < sizeof( TGA_HEADER ) )
		return false;

	TGA_HEADER* th = (TGA_HEADER*)header;

	if ( th->ImageType != 1 && th->ImageType != 2 )
	{
		cerr << error() << name << " isn't an ImageType I understand (1 or 2)" << endl;
		exit( 1 );
	}
	if ( th->PixelDepth != 8 && th->PixelDepth != 16 && th->PixelDepth != 24 && th->PixelDepth != 32 )
	{
		cerr << error() << name << " isn't an 8, 16, 24, or 32-bit Targa file" << endl;
		exit( 1 );
	}

	if ( th->ImageType == 1 )
	{
		assert( th->PixelDepth == 8 );
		assert( th->CMapStart == 0 );
	}

	if ( th->ImageType == 2 )
		assert( th->PixelDepth == 16 || th->PixelDepth == 24 || th->PixelDepth == 32 );

	if ( ( th->PixelDepth == 8 ) && ( (th->Width % 2) != 0 ) )
	{
		cerr << error() << "8-bit texture " << name <<
			"'s width isn't a multiple of two. (width is " << th->Width <<
			')' << endl;
		exit( 1 );
	}

//?	assert( th->IDLength == 0 );

	return true;
}


void
TargaBitmap::ReadBitmap( istream& input )
{
	_bitdepth = th.PixelDepth;
	_width = th.Width;
	_bufWidth = th.Width;
	_cbRow = th.Width * ( _bitdepth==8 ? 8 : 16) / 8;
	_bufHeight = _height = th.Height;

	_CMapStart = th.CMapStart;
	_CMapLength = th.CMapLength;

	pixels = (void*)malloc( _bufHeight * _cbRow );
	assert( pixels );

	switch ( _bitdepth )
	{
		case 8:
		{
			// Read the palette
			RGBQUAD* palette = (RGBQUAD*)alloca( th.CMapLength * sizeof( RGBQUAD ) );
			assert( palette );

			for ( int pal=0; pal<th.CMapLength; ++pal )
			{
				input.read( (char*)&palette[pal].r, 1 );
				input.read( (char*)&palette[pal].g, 1 );
				input.read( (char*)&palette[pal].b, 1 );
				palette[pal].Reserved = 0;
			}

			for ( int i=0; i<coloursUsed()/2; ++i )
				SWAP( palette[i], palette[ coloursUsed()-1 - i ] );

			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				uint8* pixeldata = (uint8*)pixels + (y * _width);
				input.read( (char*)pixeldata, _cbRow );
				for ( int i=0; i<width(); ++i, ++pixeldata )
					*pixeldata = coloursUsed()-1 - *pixeldata;
			}

			_calculatePaletteInfo( palette, 256 );

			break;
		}

		case 16:
		{
			_convertedPalette = NULL;
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
//				assert( _cbRow <= _width );
				uint16* pixeldata = (uint16*)pixels + (y * _width);
				input.read( (char*)pixeldata, _cbRow );
			}
			_calculatePaletteInfo( NULL, 0 );
			break;
		}

		case 24:
		{
			_convertedPalette = NULL;
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				int32* pInputLine = (int32*)alloca( _width * 3 );
				assert( pInputLine );

				input.read( (char*)pInputLine, _width * 3 );

				uint16* pixeldata = (uint16*)pixels + (y * _width);
				for ( int x=0; x<_width; ++x )
				{
					int32 r = *( (unsigned char*)pInputLine + x*3 + 0 );
					int32 g = *( (unsigned char*)pInputLine + x*3 + 1 );
					int32 b = *( (unsigned char*)pInputLine + x*3 + 2 );
					*pixeldata++ = BR_COLOUR_BGRA( r, g, b, 0 );
				}
			}
			_bitdepth = 16;
			_calculatePaletteInfo( NULL, 0 );
			break;
		}

		case 32:
		{
			_convertedPalette = NULL;
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				int32* pInputLine = (int32*)alloca( _width * 4 );
				assert( pInputLine );

				input.read( (char*)pInputLine, _width * 4 );

				uint16* pixeldata = (uint16*)pixels + (y * _width);
				for ( int x=0; x<_width; ++x )
				{
					int32 r = *( (unsigned char*)pInputLine + x*4 + 0 );
					int32 g = *( (unsigned char*)pInputLine + x*4 + 1 );
					int32 b = *( (unsigned char*)pInputLine + x*4 + 2 );
					int32 a = *( (unsigned char*)pInputLine + x*4 + 3 );
					*pixeldata++ = RGBA_555( r, g, b, a );
				}
			}
			_bitdepth = 16;
			_calculatePaletteInfo( NULL, 0 );
			break;
		}

		default:
		{
			cerr << error() << "Internal: bit depth " << _bitdepth << " not coded" << endl;
			exit( 1 );
		}
	}
}


TargaBitmap::TargaBitmap( istream& input, const char* szBitmapName ) : Bitmap()
{
	name = strdup( szBitmapName );
	assert( name );

	input.read( (char*)&th, sizeof( th ) );
	if ( !Validate( &th, sizeof( th ) ) )
	{
		cerr << szBitmapName << " is not a TGA format" << endl;
		return;
	}

	input.seekg( th.IDLength, ios::cur );

	bDebug && cout << "filename: " << szBitmapName << endl;
	ReadBitmap( input );

	if ( bitdepth() == 16 )
	{
		foreach_pixel( Pixel_TransparentRemap );
		if ( ts == TARGET_PLAYSTATION )
			foreach_pixel( Pixel_RGB_BRG );
	}

	loadColourCycles();

	if ( bitdepth() == 16 && bForceTranslucent )
	{
		foreach_pixel( Pixel_ForceTranslucent );
		foreach_pixel( Pixel_VerifyTranslucent );
	}
}


void
TargaBitmap::save( ostream& s ) const
{
	assert( 0 );

	static TGA_HEADER	th =
	{
        0,	// char IDLength;
        0,	// char ColorMapType;
        2,	// char ImageType;
		0,	// int16 CMapStart;
		0,	// int16 CMapLength;
        0,	// char CMapDepth;
        0,	// int16 XOffset;
        0,	// int16 YOffset;
        0,	// int16 Width;
        0,	// int16 Height;
        0,	// char PixelDepth;
        0	// char ImageDescriptor;
	};

	th.Width = _width;
	th.Height = _height;
	th.PixelDepth = _bitdepth;

	s.write( (char*)&th, sizeof( th ) );

	for ( int y = th.Height-1; y >= 0; --y )
	{
		uint16* pixeldata = (uint16*)pixels + (y * _bufWidth);
		s.write( (char*)pixeldata, th.Width * sizeof( uint16 ) );
	}
}


void
TargaBitmap::save( const char* filename ) const
{
	assert( 0 );

	ofstream* output = new ofstream( filename, ios::binary );
	assert( output );

	save( *output );

	delete( output );
}


// for palette reduction
static int
findPaletteEntry( uint16* palette, int nEntries, uint16 colour )
{
	for ( int i=0; i<nEntries; ++i )
	{
		if ( palette[ i ] == colour )
			return i;
	}

	return -1;
}


static bool
addToPalette( uint16* palette, int& idxColour, uint16 colour )
{
	for ( int i=0; i<idxColour; ++i )
	{
		if ( palette[ i ] == colour )
			return true;
	}

	if ( idxColour < 256 )
	{
		palette[ idxColour++ ] = colour;
		return true;
	}

	return false;
}


// TODO: Move to "bitmap" class
void
TargaBitmap::reduceTo16()
{
	for ( int y = 0; y < _bufHeight; ++y )
	{
		int32* pixeldata32 = (int32*)pixels + (y * _width);
		uint16* pixeldata16 = (uint16*)pixels + (y * _width);
		assert( (void*)pixeldata16 <= (void*)pixeldata32 );

		for ( int x=0; x < _width; ++x )
		{
			int32 r = ( *pixeldata32 >>  0 ) & 0xFF;
			int32 g = ( *pixeldata32 >>  8 ) & 0xFF;
			int32 b = ( *pixeldata32 >> 16 ) & 0xFF;
			int32 a = ( *pixeldata32 >> 24 ) & 0xFF;

			*pixeldata16 = RGBA_555( r, g, b, a );
			++pixeldata16, ++pixeldata32;
//			printf( "r=%x g=%x b=%x a=%d  ", r, g, b, a );
		}
	}

	// Try to shrink it down to a 4 or 8 bit texture
	uint16* palette = (uint16*)alloca( 256 * sizeof( uint16 ) );
	assert( palette );
	int idxColour = 0;

	for ( int yCoord = 0; yCoord < _bufHeight; ++yCoord )
	{
		uint16* pixeldata16 = (uint16*)pixels + (yCoord * _width);

		for ( int x=0; x < _width; ++x )
		{
			if ( !addToPalette( palette, idxColour, *pixeldata16 ) )
				return;		// Couldn't fit
			++pixeldata16;
		}
	}
	_bitdepth = 16;
	_cbRow = _width * _bitdepth/8;
	_calculateArea();

	if ( idxColour < 256 )
		reduceTo8( palette, idxColour );

#if 0
	if ( idxColour < 16 )
		reduceTo4( palette, idxColour );
#endif
}


void
TargaBitmap::reduceTo8( uint16* palette, int nColours )
{
	assert( bitdepth() == 16 );

	_convertedPalette = (uint16*)malloc( nColours * sizeof( uint16 ) );
	assert( _convertedPalette );
	memcpy( _convertedPalette, palette, nColours * sizeof( uint16 ) );

	_CMapLength = nColours;

	for ( int y = 0; y < _bufHeight; ++y )
	{
		uint16* pixeldata16 = (uint16*)pixels + (y * _width);
		uint8* pixeldata8 = (uint8*)pixels + (y * _width);
		assert( (void*)pixeldata8 <= (void*)pixeldata16 );

		for ( int x=0; x < _width; ++x )
		{
            int tempPixelData = findPaletteEntry( _convertedPalette, nColours, *pixeldata16 );
			assert( *pixeldata8 != -1 );
			*pixeldata8 = tempPixelData;
			++pixeldata8, ++pixeldata16;
		}
	}

	_bitdepth = 8;
	_cbRow = _width * _bitdepth/8;

	checkForTransparentColour();

	_calculateArea();
}


void
TargaBitmap::reduceTo4( uint16* palette, int nColours )
{
	assert( bitdepth() == 8 );

	_bitdepth = 4;
	_calculateArea();
}

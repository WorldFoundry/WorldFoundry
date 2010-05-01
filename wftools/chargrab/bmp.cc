//==============================================================================
// bmp.cc: Copyright (c) 1995-1999, World Foundry Group  
// by William B. Norris IV
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
// 08 Dec 95	WBNIV	Windows .bmp-specific instance of Bitmap class created
// 10 Apr 97	WBNIV	Allows user to specify the transparent colour
//==============================================================================

#include <stdlib.h>
#include <pigsys/assert.hp>
#include <string>
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
extern bool bPowerOf2Size;
extern bool bForceTranslucent;
extern uint16 colTransparent;


WindowsBitmap::~WindowsBitmap()
{
}


WindowsBitmap::WindowsBitmap( const int width, const int height,
	const uint16 bgColor ) : Bitmap( width, height, bgColor )
{
}


bool
WindowsBitmap::Validate( void* header, size_t size )
{
	if ( size < sizeof( BmpHeader ) )
		return false;

	if ( BmpHeader.fh.bfType != 'MB' )
		return false;

	if ( BmpHeader.ih.biSize != sizeof( BITMAPINFOHEADER ) )
		return false;

	if ( BmpHeader.ih.biBitCount == 4 )
	{
		if ( (BmpHeader.ih.biWidth % 4) != 0 )
		{
			cerr << "4-bit texture " << name << "'s width isn't a multiple of 4" << endl;
			return false;
			exit( 1 );
		}
	}
	else if ( BmpHeader.ih.biBitCount == 8 )
	{
		if ( (BmpHeader.ih.biWidth % 2) != 0 )
		{
			cerr << "8-bit texture " << name << "'s width isn't a multiple of 2" << endl;
			return false;
			exit( 1 );
		}
	}
	else if ( BmpHeader.ih.biBitCount == 24 )
	{
		if ( (BmpHeader.ih.biWidth % 2*3) != 0 )
		{
			cerr << "24-bit texture " << name << "'s width isn't a multiple of 2" << endl;
			return false;
			exit( 1 );
		}
	}

	_width = BmpHeader.ih.biWidth;
	_bufWidth = BmpHeader.ih.biWidth;
	_bitdepth = BmpHeader.ih.biBitCount;
	_cbRow = BmpHeader.ih.biWidth * ( _bitdepth==8 ? 8 : 16) / 8;
	_bufHeight = _height = BmpHeader.ih.biHeight;

	if ( _bitdepth != 4 && _bitdepth != 8 && _bitdepth != 24 )
	{
		cerr << error() << name << " isn't a 4, 8, or 24-bit BMP file" << endl;
		return false;
		exit( 1 );
	}

	return true;
}


void
WindowsBitmap::ReadBitmap( istream& input )
{
	_CMapStart = 0;
	_CMapLength = BmpHeader.ih.biClrUsed ? BmpHeader.ih.biClrUsed : (1<<_bitdepth);

	pixels = (void*)malloc( _bufHeight * _cbRow );
	assert( pixels );

	if ( bitdepth() == 4 || bitdepth() == 8 )
	{
		palette = (RGBQUAD*)alloca( _CMapLength * sizeof( RGBQUAD ) );
		assert( palette );
		input.read( (char*)palette, _CMapLength * sizeof( RGBQUAD ) );
		for ( int i=0; i<_CMapLength; ++i )
			assert( palette[i].Reserved == 0 );

		if ( bitdepth() == 8 )
		{	// Remap palette upside down
			int cu = coloursUsed();
			for ( int i=0; i<cu/2; ++i )
				SWAP( palette[i], palette[ cu-1 - i ] );
		}
	}
	else
	{
		palette = NULL;
	}

	input.seekg( BmpHeader.fh.bfOffBits );

	switch ( _bitdepth )
	{
		case 4:
		{
//			for ( int y=0; y < _bufHeight; ++y )
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				uint8* pixeldata = (uint8*)pixels + (y * _cbRow );
				input.read( (char*)pixeldata, _cbRow );
				for ( int i=0; i<_cbRow; ++i )
				{
					uint8 pixel = *pixeldata;
					uint8 lnibble = pixel & 0x0F;
					uint8 unibble = (pixel & 0xF0) >> 4;
					pixel = (lnibble<<4) | unibble;
					*pixeldata++ = pixel;
				}
			}
			_calculatePaletteInfo( palette, 1<<_bitdepth );
			break;
		}

		case 8:
		{
//			for ( int y=0; y < _bufHeight; ++y )
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				uint8* pixeldata = (uint8*)pixels + (y * _cbRow );
				input.read( (char*)pixeldata, _cbRow );
				for ( int i=0; i<width(); ++i, ++pixeldata )
					*pixeldata = coloursUsed()-1 - *pixeldata;
			}
			_calculatePaletteInfo( palette, 1<<_bitdepth );
			break;
		}

		case 24:
		{
			_convertedPalette = NULL;
//			for ( int y=0; y<_bufHeight; ++y )
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				int32* pInputLine = (int32*)_alloca( _width * 3 );
				assert( pInputLine );

				input.read( (char*)pInputLine, _width * 3 );

				uint16* pixeldata = (uint16*)pixels + (y * _width);
				for ( int x=0; x<_width; ++x )
				{
					int32 r = *( (unsigned char*)pInputLine + x*3 + 0 );
					int32 g = *( (unsigned char*)pInputLine + x*3 + 1 );
					int32 b = *( (unsigned char*)pInputLine + x*3 + 2 );
					*pixeldata++ = BR_COLOUR_BGR( r, g, b );
				}
			}
			_bitdepth = 16;
			_calculatePaletteInfo( NULL, 0 );
			break;
		}
	}
}


WindowsBitmap::WindowsBitmap( istream& input, const char* szBitmapName ) : Bitmap()
{
	name = strdup( szBitmapName );
	assert( name );

	input.read( (char*)&BmpHeader, sizeof( BmpHeader ) );

	if ( !Validate( &BmpHeader, sizeof( BmpHeader ) ) )
	{
		cerr << szBitmapName << " is not a BMP format" << endl;
		return;
	}

	ReadBitmap( input );
	assert( bitdepth() == 4 || bitdepth() == 8 || bitdepth() == 16 );

	if ( bitdepth() == 16 )
	{
		if ( colTransparent )
			foreach_pixel( Pixel_TransparentRemap );
		if ( ts == TARGET_PLAYSTATION )
			foreach_pixel( Pixel_RGB_BRG );
	}

	if ( bPowerOf2Size )
		checkAcceptablePowerof2();

	loadColourCycles();

	if ( bitdepth() == 16 && bForceTranslucent )
	{
		foreach_pixel( Pixel_ForceTranslucent );
#if !defined( NDEBUG )
		foreach_pixel( Pixel_VerifyTranslucent );
#endif
	}
}


void
WindowsBitmap::save( const char* filename ) const
{
	cerr << error() << "BMP save() not implemented" << endl;
	exit( 1 );
}

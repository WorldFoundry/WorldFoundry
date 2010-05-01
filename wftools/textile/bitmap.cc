//==============================================================================
// bitmap.cc
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
// ?? ??? 95	WBNIV	Original version
// 06 Dec 95	WBNIV	Added 8-bit Targa support
// 07 Dec 95	WBNIV	Paletted image support
// 10 Apr 97	WBNIV	Allows user to specify the transparent colour
// 20 Jun 97	WBNIV	Added force translucent
//==============================================================================

#include "global.hp"

#include <stdlib.h>
#include <pigsys/assert.hp>
#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include "types.h"
#include "allocmap.hp"
#include "bitmap.hp"
#include "textile.hp"
#include "profile.hp"

extern bool bShowAlign;
extern bool bFrame;
extern bool bGlobalError;
extern bool bVerbose;
extern bool bPowerOf2Size;
extern TargetSystem ts;
extern uint16 colTransparent;
extern char szColourCycle[];
extern char szOutDir[];
extern bool bForceTranslucent;

//#include "memcheck.h"

Bitmap::Bitmap()
{
	pixels = NULL;
	_convertedPalette = NULL;
	_CMapStart = 0;
	_CMapLength = 0;
	_startColour = _endColour = -1;
	name = NULL;
	_bHasTransparentColour = false;
	_subx = _suby = -1;
	_area = -1;
	xPal = yPal = -1;
	_idxPal = -1;
	_nColourCycles = 0;
	_bPowerOf2Size = ::bPowerOf2Size;
}


Bitmap::Bitmap( const int width, const int height, const uint16 bgColor )
{
	_bufWidth = _width = width;
	_cbRow = _bufWidth * sizeof( uint16 );
	_bufHeight = _height = height;
	_bitdepth = 16;

	pixels = (uint16*)malloc( _cbRow * (_height+1) );
	assert( pixels );

	memset( pixels, bgColor, _cbRow * (_height+1) );

	xPal = yPal = -1;
	_idxPal = -1;
	_convertedPalette = NULL;
	_CMapStart = 0;
	_CMapLength = 0;
	_startColour = _endColour = -1;
	_nColourCycles = 0;

	name = NULL;

	_bPowerOf2Size = ::bPowerOf2Size;
	_calculateArea();
}


Bitmap::Bitmap( const Bitmap& bm )
{	// create bitmap from palette
	pixels = malloc( bm.coloursUsed() * sizeof( uint16 ) );
	assert( pixels );
	memcpy( pixels, bm._convertedPalette, bm.coloursUsed() * sizeof( uint16 ) );

	_bufWidth = _width = bm.coloursUsed();
	_cbRow = _bufWidth * sizeof( uint16 );
	_bufHeight = _height = 1;
	_bitdepth = 16;

	_convertedPalette = NULL;
	_CMapStart = 0;
	_CMapLength = 0;
	_startColour = _endColour = -1;

	name = strdup( bm.name );
	_bHasTransparentColour = checkForTransparentColour();
	_subx = _suby = -1;
	_area = -1;
	xPal = yPal = -1;
	_idxPal = -1;

	_bPowerOf2Size = bm._bPowerOf2Size;

	_nColourCycles = bm._nColourCycles;
	assert( _nColourCycles <= _MAX_COLOUR_CYCLES );
	for ( int i=0; i<_nColourCycles; ++i )
		_colourCycle[i] = bm._colourCycle[i];

	_calculateArea();
}


// swap R & B components of a 5:5:5 pixel
void
Bitmap::Pixel_RGB_BRG( uint16& pixel )
{
	uint16 r = BR_BLU_S( pixel );
	uint16 g = BR_GRN_S( pixel );
	uint16 b = BR_RED_S( pixel );
	uint16 a = BR_ALPHA_S( pixel );

	pixel = BR_COLOUR_BGRA( r, g, b, a );
}


void
Bitmap::Pixel_Print( uint16& colour )
{
	cout << colour << ' ';
}


void
Bitmap::Pixel_NOP( uint16& )
{
}


void
Bitmap::Pixel_TransparentRemap( uint16& colour )
{
	// Check for transparent colour remapping
	if ( colour == colTransparent )
		colour = 0;
	else if ( colour == 0 )
		colour = TRANSLUCENT;		// Set alpha bit (only)
}


void
Bitmap::Pixel_ForceTranslucent( uint16& colour )
{
	colour |= TRANSLUCENT;
}


void
Bitmap::Pixel_VerifyTranslucent( uint16& colour )
{
	assert( ( colour == 0 ) || ( colour & TRANSLUCENT ) );
}


#if 0
inline uint16
RGBA_555( int32 r, int32 g, int32 b, int32 a )
{
	// Short-circuit easy case (no calculations required)
	if ( a < 85 )
		return 0;

	r >>= 3;
	g >>= 3;
	b >>= 3;
	a = a > 170;
	return uint16( (a<<15) | (b<<10) | (g<<5) | (r) );
}
#endif



void
Bitmap::foreach_pixel( fnPixel fn )
{
	assert( bitdepth() == 16 );

	for ( int y=0; y<height(); ++y )
	{
		uint16* dest = ((uint16*)pixels) + y * (_cbRow/2);;

		assert( width() * 2 == _cbRow );
		for ( int x=0; x<width(); ++x )
		{
			assert( dest >= pixels );
			assert( (char *)dest < (char *)pixels + _cbRow * _height );
			fn( *dest );
			++dest;
		}
	}
}


bool
Bitmap::hasTransparentColour() const
{
	return _bHasTransparentColour;
}


bool
Bitmap::Validate( void* header, size_t )
{
	return true;
}


void
Bitmap::ReadBitmap( istream& input )
{
}


void
Bitmap::removeColourCycles()
{
	_nColourCycles = 0;
}


void
Bitmap::loadColourCycles()
{
	assert( _nColourCycles == 0 );
	//cout << "loadColourCycles() name=[" << name << ']' << endl;
	if ( !name )
		return;

	int _nCycles = 0;
	_nCycles = GetPrivateProfileInt( name, "nCycles", _nCycles, szColourCycle );
	//cout << "_nCycles = " << _nCycles << endl;
	for ( int i=0; i<_nCycles; ++i )
	{
		char szSection[ PATH_MAX ];
		sprintf( szSection, "%s:%d", name, i );

		int _start = -1;
		_start = GetPrivateProfileInt( szSection, "StartColour", _start, szColourCycle );

		int _end = -1;
		_end = GetPrivateProfileInt( szSection, "EndColour", _end, szColourCycle );

		char szSpeed[ 128 ] = "0.0";
		GetPrivateProfileString( szSection, "Speed", szSpeed, szSpeed, sizeof( szSpeed ), szColourCycle );
		float _speed = atof( szSpeed );

		if ( ( _start == -1 ) || ( _end == -1  ) || ( _speed == 0.0 ) )
			{ // no colour cycle information
			cout << "Incomplete colour cycle specification for " << szSection << endl;
			return;
			}

		_colourCycle[ _nColourCycles++ ] = ColourCycle( _start, _end, _speed );
		assert( _nColourCycles <= _MAX_COLOUR_CYCLES );
	}
}


void
Bitmap::_calculatePaletteInfo( RGBQUAD* palette, int nEntries )
{
	assert( nEntries == 0 || nEntries == 16 || nEntries == 256 );

	if ( nEntries )
	{
        if ( nEntries == 16 )
		{
            _startColour = 0;
            _endColour = 15;
		}
        else if ( nEntries == 256 )
		{
			int _histogram[ 256 ];
			memset( _histogram, 0, sizeof( _histogram ) );

			// Do a histogram of the pixels
			uint8* pPixels = (uint8*)pixels;
			for ( int i=0; i < _cbRow * _height; ++i )
				++_histogram[ *pPixels++ ];

			// Find first non-used palette entry
			for ( int paletteIndex=nEntries-1; paletteIndex >= 0; --paletteIndex )
			{
				if ( _histogram[ paletteIndex ] && _endColour == -1 )
					_endColour = paletteIndex;

				if ( _histogram[ paletteIndex ] )
					_startColour = paletteIndex;
			}
		}

		_CMapLength = _endColour+1;

//		*textilelog << name << " [" << _startColour << ".." << _endColour << ']' << endl;

		// convert palette to destination format
		_convertedPalette = (uint16*)malloc( _CMapLength * sizeof(uint16) );
		assert( _convertedPalette );

		for ( int i=0; i<_CMapLength; ++i )
		{
			_convertedPalette[i] = BR_COLOUR_RGB_555( palette[i].r, palette[i].g, palette[i].b );
			if ( colTransparent )
			{	// Something other than black is specified as transparent
				if ( _convertedPalette[i] == colTransparent )
					_convertedPalette[i] = 0x000000;
				else if ( _convertedPalette[i] == 0 )
					_convertedPalette[i] = TRANSLUCENT;		// Set alpha bit (only)
			}
		}
	}
	else
	{
		//xPal = yPal = -1;
		_bHasTransparentColour = checkForTransparentColour();
	}

	_calculateArea();
}


Bitmap::~Bitmap()
{
	if ( name ) free( name );
	if ( _convertedPalette ) free( _convertedPalette );		// Palette optional--not all bitmaps have one
	assert( pixels );
	free( pixels );
}


void
Bitmap::_calculateArea()
{
	_area = width() * height() * bitdepth() / 16;
}


bool
Bitmap::operator>( const Bitmap& bm2 ) const
{
	assert( _area > 0 );
	assert( bm2._area > 0 );

	//printf( "%s=%d  %s=%d\n", name, _area, bm2.name, bm2._area );
	return bm2._area < _area;
}


bool
Bitmap::checkForTransparentColour() const
{
	assert( bitdepth() == 16 );
	// Only valid for 16-bit entries.  This works because either the image is a
	// 16-bit texture or we've passed in the palette, which are 16-bit colour arrays

	switch ( bitdepth() )
	{
		case 16:
		{
			for ( int y = 0; y < height(); ++y )
			{
				uint16* pixeldata = (uint16*)pixels + (y * _bufWidth);
				for ( int x=0; x<width(); ++x )
					if ( *pixeldata & TRANSLUCENT )
					{
						printf( "Found a translucent image\n" );
						return true;
					}
			}
			break;
		}

		default:
			assert( 0 );
			break;
	}

	return false;
}


void
Bitmap::hline( int x, int y, int width )
{
	if ( width )
	{
		uint16* pixeldata = (uint16*)pixels + (y * _bufWidth) + x;
		//cout << "x " << x << " y " << y << " width " << width << endl;
		memset( pixeldata, ~0, width * sizeof( uint16 ) );
	}
}


void
Bitmap::vline( int x, int y, int height )
{
	uint16* pixeldata = (uint16*)pixels + (y * _bufWidth) + x;
	for ( int yy=0; yy<height; ++yy, pixeldata += _bufWidth )
		*pixeldata = ~0;
}


void
Bitmap::box( int x, int y, int width, int height )
{
	for ( int yy=0; yy<height; ++yy )
		hline( x, y+yy, width );
}


void
Bitmap::frame( int x1, int y1, int width, int height )
{
	int x2 = x1 + width - 1;
	int y2 = y1 + height - 1;

	hline( x1, y1, width );
	vline( x1, y1, height );
	hline( x1, y2, width );
	vline( x2, y1, height );
}


void
Bitmap::save( const char* filename ) const
{
	extern bool bFlipYOut;

#pragma pack( 1 )
	typedef struct
	{
        char IDLength;
        char ColorMapType;
        char ImageType;
		int16 CMapStart;
		int16 CMapLength;
        char CMapDepth;
        int16 XOffset;
        int16 YOffset;
        int16 Width;
        int16 Height;
        char PixelDepth;
        char ImageDescriptor;
	} TGA_HEADER;
#pragma pack()

	static TGA_HEADER th =
	{
        0,	// char IDLength;
        0,	// char ColorMapType;
        2,	// char ImageType;		only write out 16-bit images
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

	// TODO: remove szOutDir, make a function which does this, and pass the result in as the parameter to this function
	char szOutputBitmap[ PATH_MAX ];
	sprintf( szOutputBitmap, "%s/%s", szOutDir, filename );
	if ( bVerbose )
		cout << "Opening \"" << szOutputBitmap << "\"" << endl;
	FILE* fp = fopen( szOutputBitmap, "wb" );
	assert( fp );

	fwrite( &th, sizeof( th ), 1, fp );

	// TODO: this can be written better
	if ( bFlipYOut )
	{
		for ( int y = 0; y < th.Height; ++y )
		{
			uint16* pixeldata = (uint16*)pixels + (y * _bufWidth);
			int cbWritten = fwrite( pixeldata, sizeof( *pixeldata ), th.Width, fp );
			assert( cbWritten == th.Width );
		}
	}
	else
	{
		for ( int y = th.Height-1; y >= 0; --y )
		{
			uint16* pixeldata = (uint16*)pixels + (y * _bufWidth);
			int cbWritten = fwrite( pixeldata, sizeof( *pixeldata ), th.Width, fp );
			assert( cbWritten == th.Width );
		}
	}

	// Output colour cycling information
	// TODO: align?
	for ( int i=0; i<_nColourCycles; ++i )
	{
		int cbWritten;
		const ColourCycle& cc = _colourCycle[ i ];
		cbWritten = fwrite( &cc._start, sizeof( int32 ), 1, fp );
		assert( cbWritten == 1 );
		cbWritten = fwrite( &cc._end, sizeof( int32 ), 1, fp );
		assert( cbWritten == 1 );
		int32 speed = int(cc._speed * 65536.0);		// convert to fixed point
		cbWritten = fwrite( &speed, sizeof( int32 ), 1, fp );
		assert( cbWritten == 1 );
	}

	fclose( fp );
}


bool
Bitmap::sameBitmap( int x, int y, const Bitmap* bmCheck ) const
{
	assert( bitdepth() == 16 );

	if ( _nColourCycles != bmCheck->_nColourCycles )
		return false;

	for ( int i=0; i<_nColourCycles; ++i )
	{
		if ( _colourCycle[i] != bmCheck->_colourCycle[i] )
			return false;
	}

	uint16* lineSrc = (uint16*)pixels + _cbRow/2 * y + x;
	uint16* lineCheck = (uint16*)bmCheck->pixels;

	for ( int yy=y; yy<y+bmCheck->height(); ++yy )
	{
		if ( memcmp( lineSrc, lineCheck, bmCheck->_cbRow ) != 0 )
			return false;
		lineSrc += _cbRow/2;
		lineCheck = (uint16*)( (int8*)lineCheck + bmCheck->_cbRow );
	}

	return true;
}


bool
Bitmap::find( const Bitmap* bmAdd, AllocationMap& allocated, int& x, int& y ) const
{
	for ( y=0; y <= height() - bmAdd->height(); y += allocated.yAlign() )
	{
		for ( x=0; x <= width() - bmAdd->width(); x += allocated.xAlign() )
		{
			if ( sameBitmap( x, y, bmAdd ) )
				return true;
		}
	}

	return false;
}


bool
Bitmap::texture_fit( Bitmap* bmAdd, AllocationMap& allocated,
	const char* szRoomName, const char* szTextureType, int i )
{
	int x, y;

	assert( bmAdd->name );

	if ( find( bmAdd, allocated, x, y ) )
		{
		bmAdd->_subx = x;
		bmAdd->_suby = y;
		return true;
	}

	bool bFoundSlot = allocated.find( this, bmAdd, i, x, y );
	if ( !bFoundSlot )
	{
		cout << error() << "couldn't fit " << szTextureType <<  " \"" << bmAdd->name << "\" in room \"" << szRoomName << "\"" << endl;
		*textilelog << error() << "couldn't fit " << szTextureType <<  " \"" << bmAdd->name << "\" in room \"" << szRoomName << "\"" << endl;
		bGlobalError = true;
		//?bmAdd->_subx = bmAdd->_suby = -1;
	}
	else
	{
		copy( bmAdd, x * allocated.xAlign() / allocated.k(), y * allocated.yAlign() );
		if ( bFrame || bShowAlign )
		{
			int x1 = x * allocated.xAlign();
			int y1 = y * allocated.yAlign();

			if ( bFrame )
				frame( x*allocated.xAlign(), y1, bmAdd->width(), bmAdd->height() );

			if ( bShowAlign )
			{	// Mark alignment slop
				box( x1 + bmAdd->width(), y1,
					round( bmAdd->width(), allocated.xAlign() ) - bmAdd->width(),
					round( bmAdd->height(), allocated.yAlign() ) - bmAdd->height() );
			}
		}

		bmAdd->_subx = x * allocated.xAlign() / (16/4);
		bmAdd->_suby = y * allocated.yAlign();
	}

	return bFoundSlot;
}


void
Bitmap::crop( const int width, const int height )
{
	_width = width;
	_height = height;
}


void
Bitmap::copy( const Bitmap* srcBitmap, const int xDest, const int yDest )
{
	assert( bitdepth() == 16 );

	for ( int i=0; i<srcBitmap->_nColourCycles; ++i )
	{
		_colourCycle[ _nColourCycles++ ] = srcBitmap->_colourCycle[ i ];
		assert( _nColourCycles <= _MAX_COLOUR_CYCLES );
	}

	assert( ( srcBitmap->_cbRow % 2 ) == 0 );
	switch ( srcBitmap->bitdepth() )
	{
		case 4:
		case 8:
		{
			for ( int y=0; y<srcBitmap->height(); ++y )
			{
				memcpy( ((uint16*)pixels) + (y + yDest) * _cbRow/2 + xDest,
					(int8*)srcBitmap->pixels + y * srcBitmap->_cbRow,
					srcBitmap->_cbRow );
			}
			break;
		}

		case 16:
		{
			for ( int y=0; y<srcBitmap->height(); ++y )
			{
				uint16* dest = ((uint16*)pixels) + (y + yDest) * (_cbRow/2) + xDest;

				memcpy( dest,
					((uint16*)srcBitmap->pixels) + y * srcBitmap->width(),
					srcBitmap->_cbRow );
			}
		}
	}
}


#define powerof2( i )	( (i==16) || (i==32) || (i==64) || (i==128) || (i==256) || (i==512) || (i==1024) )

bool
Bitmap::checkAcceptablePowerof2()
{
	bool ok = true;

	assert( name );

	if ( !powerof2( width() / (16/bitdepth() ) ) )
	{
		cerr << error() << "Texture " << name <<
			"'s width is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  endl;
		*textilelog << error() << "Texture " << name <<
			"'s width is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  endl;
		ok = false;
		bGlobalError = true;
	}

	if ( !powerof2( height() ) )
	{
		cerr << error() << "Texture " << name <<
			"'s height is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  endl;
		*textilelog << error() << "Texture " << name <<
			"'s height is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  endl;
		ok = false;
		bGlobalError = true;
	}

	return ok;
}

//==============================================================================
// palette.cc: Copyright (c) 1996-1999, World Foundry Group  
// By William B. Norris IV
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

#include <stdio.h>
#include <pigsys/assert.hp>
#include <stdlib.h>
#include <fstream>
#include <iomanip>
using namespace std;

#include "textile.hp"
#include "palette.hp"
extern bool bDebug;

extern int pal_x_align;
extern int pal_y_align;


template <class T>
inline T round( T a, int alignment )
{
	T tmp = a;
	tmp += alignment - 1;
	tmp /= alignment;
	tmp *= alignment;
	return tmp;
}


SuperPalette::SuperPalette( const int width, const int height )
{
	assert( width > 0 );
	_width = width;
	assert( height > 0 );
	_height = height;

	_paletteAllocation = new int[ height ];
	assert( _paletteAllocation );
	for ( int i=0; i < height; ++i )
		_paletteAllocation[ i ] = 0;
	_paletteAllocationHeight = 0;

	_thePalette = new uint16[ height * width ];
	assert( _thePalette );
	memset( _thePalette, 0, height * width * sizeof( uint16 ) );
}


SuperPalette::~SuperPalette()
{
	delete[] _thePalette;
	delete[] _paletteAllocation;
}


int
SuperPalette::width() const
{
	return _width;
}


int
SuperPalette::height() const
{
	return _height;
}


ostream& operator<<( ostream& s, SuperPalette& pal )
{
	// kludge:
	TargaBitmap* bm = new TargaBitmap( pal._width, pal._height, 0 );
	assert( bm );

	memcpy( bm->pixels, pal._thePalette, pal._width * pal._height * sizeof( uint16 ) );

	bm->save( s );

	delete bm;

	return s;
}


void
SuperPalette::add( Bitmap& bm, int& xPal, int& yPal )
{
	assert( bm.bitdepth() <= 8 );

#define BR_COLOUR_RGB_555( r, g, b ) \
	( ((uint)(r) & 0xF8) << (10-3) | \
	((uint)(g) & 0xF8) << (5-3) | \
	((uint)(b) & 0xF8) >> 3 )

	// convert palette to destination format for comparison
	uint16 testPalette[ 256 ];
	for ( int i=0; i<bm._CMapLength; ++i )
	{
		testPalette[i] = BR_COLOUR_RGB_555( bm.palette[i].r,
			bm.palette[i].g, bm.palette[i].b );
	}

	assert( pal_y_align == 1 );		// Program can't handle other settings yet

	// First look for this new palette as a subset of an existing palette
	for ( int y=0; y<_paletteAllocationHeight; ++y )
	{
//		for ( int x=0; x<=_width-bm._CMapLength; x += pal_x_align*sizeof(uint16) )
		for ( int x=0; x<=_width-bm._CMapLength; x += 64 )
		{
			if ( memcmp( _thePalette + y * _width + x, testPalette, bm._CMapLength * sizeof( uint16 ) ) == 0 )
			{
				yPal = y;
				xPal = x;
				bDebug && printf( "Found a subpalette @ (%d,%d) for texture %s\n", xPal, yPal, bm.name );
				*log << "Found a subpalette @ (" << xPal << "," << yPal << ") len=" << bm.coloursUsed() << " for texture " << bm.name << endl;
				return;
			}
		}
	}

	// If an existing palette can't be found, find space to allocate a new one
	for ( y=0; y<_height; ++y )
	{
		if ( bm._CMapLength <= _width - _paletteAllocation[ y ] )
		{	// Found an empty space
			yPal = y;
			xPal = _paletteAllocation[ y ];
			_paletteAllocation[ y ] += bm._CMapLength+1;
			_paletteAllocation[ y ] = round( _paletteAllocation[ y ], pal_x_align*2 );

			bDebug && printf( "Adding a new palette @ (%d,%d) len=%d for texture %s\n", xPal, yPal, bm._CMapLength, bm.name );
			*log << "Adding a new palette @ (" << xPal << "," << yPal << ") len=" << bm.coloursUsed() << " for texture " << bm.name << endl;

			assert( (_paletteAllocation[ y ] % (pal_x_align)) == 0 );

			memcpy( _thePalette + yPal * _width + xPal, testPalette, bm._CMapLength * sizeof( uint16 ) );

			if ( y > _paletteAllocationHeight )
				_paletteAllocationHeight = y;

			return;
		}
	}

	cout << error() << "Couldn't fit palette for " << bm.name << " (" << bm.coloursUsed() << " colours)" << endl;
	*log << error() << "Couldn't fit palette for " << bm.name << " (" << bm.coloursUsed() << " colours)" << endl;
}


void
SuperPalette::print() const
{
	*log << "Palette allocation map" << endl;

	for ( int y=0; y<_height; ++y )
	{
		*log << setw(2) << y << ' ';
		for ( int x=0; x<_width; ++x )
		{
			*log << ( x < _paletteAllocation[y] ? 'X' : '.' );
		}
		*log << endl;
	}
}

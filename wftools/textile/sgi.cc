//==============================================================================
// sgi.cc
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

#include <pigsys/assert.hp>
#include <stdlib.h>
#include <malloc.h>
#include <string>
#include "bitmap.hp"
#include "sgi.h"

extern bool bDebug;
extern TargetSystem ts;
extern bool bForceTranslucent;

SgiBitmap::SgiBitmap( const int width, const int height,
	const uint16 bgColor ) : Bitmap( width, height, bgColor )
{
	assert( sizeof( SGI_Header ) == 512 );
}


SgiBitmap::~SgiBitmap()
{
}


uint32
utl_flipLW( uint32 v )
{
	register uint32	val = 0;
	register uint8	tmp;

	tmp = v >> 24;
	val = (uint32)tmp;

	tmp = v >> 16;
	val |= (uint32)tmp << 8;

	tmp = v >> 8;
	val |= (uint32)tmp << 16;

	tmp = v >> 0;
	val |= (uint32)tmp << 24;

	return( val );
}


uint16
utl_flipSW( uint16 v )
{
	uint32 val = 0;
	register uint8	tmp;

	tmp = v >> 8;
	val = (uint32)tmp;

	tmp = v >> 0;
	val |= (uint32)tmp << 8;

	return( (uint16)val );
}



bool
SgiBitmap::Validate( void* header, size_t size )
{
	assert( sizeof( SGI_Header ) == 512 );

	if ( size < sizeof( SGI_Header ) )
		return false;

	SGI_Header* sh = (SGI_Header*)header;

	sh->magic = utl_flipSW( sh->magic );
	sh->dimension = utl_flipSW( sh->dimension );
	sh->ixsize = utl_flipSW( sh->ixsize );
	sh->iysize = utl_flipSW( sh->iysize );
	sh->zsize = utl_flipSW( sh->zsize );
	sh->pixmin = utl_flipLW( sh->pixmin );
	sh->pixmax = utl_flipLW( sh->pixmax );
	sh->colormap = utl_flipLW( sh->colormap );

	if ( sh->magic != 474 )
		return false;

	assert( sh->storage == 0 );
	assert( sh->bpc == 1 );
	assert( sh->dimension == 2 );
	assert( sh->pixmin == 0 );
	assert( sh->pixmax == 255 );
	assert( sh->colormap == 0 );

	return true;
}


uint16
Greyscale_RGB555( unsigned char grey )
{
	int r, g, b, a;
	grey >>= 3;
	r = g = b = grey;
	a = 0;
	return uint16( (a<<15) | (b<<10) | (g<<5) | (r) );
}


inline uint16
RGB_555( int32 r, int32 g, int32 b )
{
	r >>= 3;
	g >>= 3;
	b >>= 3;
	return uint16( (0<<15) | (b<<10) | (g<<5) | (r) );
}


void
SgiBitmap::ReadBitmap( istream& input )
{
	_bitdepth = 16;

	_bufWidth = _width = sh.ixsize;
	_bufHeight = _height = sh.iysize;
	_cbRow = _width * 2;

	_CMapStart = 0;	//th.CMapStart;
	_CMapLength = 255;	//th.CMapLength;

	pixels = (void*)malloc( _bufHeight * _cbRow );
	assert( pixels );
	memset( pixels, RGB_555(255,255,255), _bufHeight * _cbRow );

	unsigned char* lineBuffer = (unsigned char*)alloca( sh.ixsize*sh.zsize );
	assert( lineBuffer );
	uint16* pDest = (uint16*)pixels;
	for ( int y=0; y < sh.iysize; ++y )
	{
		input.read( (char*)lineBuffer, sh.ixsize*sh.zsize );
		unsigned char* pLineBuffer = lineBuffer;
		for ( int x=0; x < sh.ixsize; ++x, ++pDest )
		{
			switch ( sh.zsize )
			{
				case 1:
				{	// Black/White
					*pDest = Greyscale_RGB555( *( pLineBuffer + x ) );
					break;
				}

				case 3:
				{	// Colour
					*pDest = RGB_555( *pLineBuffer++,
						*pLineBuffer++, *pLineBuffer++ );
					break;
				}

				case 4:
				{	// Colour + Alpha
					*pDest = RGBA_555( *pLineBuffer++, 	// r
						*pLineBuffer++, 				// g
						*pLineBuffer++, 				// b
						*pLineBuffer++ );				// alpha
					break;
				}

				default:
					break;
			}
		}
	}
}


SgiBitmap::SgiBitmap( istream& input, const char* szBitmapName ) : Bitmap()
{
	name = strdup( szBitmapName );
	assert( name );

	input.read( (char*)&sh, sizeof( sh ) );
	if ( !Validate( &sh, sizeof( sh ) ) )
	{
		cerr << szBitmapName << " is not an SGI format" << endl;
		return;
	}

	bDebug && cout << "filename: " << szBitmapName << endl;
	ReadBitmap( input );
	_calculateArea();

	if ( bitdepth() == 16 )
	{
		foreach_pixel( Pixel_TransparentRemap );
		if ( ts == TARGET_PLAYSTATION )
			foreach_pixel( Pixel_RGB_BRG );
	}

	loadColourCycles();

	if ( bForceTranslucent )
	{
		foreach_pixel( Pixel_ForceTranslucent );
		foreach_pixel( Pixel_VerifyTranslucent );
	}
}


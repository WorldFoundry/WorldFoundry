// bmp.cc
//
// by William B. Norris IV
// Copyright 1995 Cave Logic Studios, Ltd.  
//
// 08 Dec 95	WBNIV	Windows .bmp-specific instance of Bitmap class created
// 10 Apr 97	WBNIV	Allows user to specify the transparent colour

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <malloc.h>

#include <gfxfmt/types.h>
#include <gfxfmt/bitmap.hp>
#include <gfxfmt/bmp.hp>
extern bool bVerbose;
extern bool bDebug;
//extern TargetSystem ts;
//extern bool bForceTranslucent;

WindowsBitmap::~WindowsBitmap()
{
}


WindowsBitmap::WindowsBitmap( const int width, const int height,
	const uint16 bgColor ) : Bitmap( width, height, bgColor )
{
}


WindowsBitmap::WindowsBitmap( const Bitmap& bm ) : Bitmap( bm )
{
}


bool
WindowsBitmap::Validate( void* _header, size_t size, std::string msg )
{
//	assert( _header );

	BmpHeader* header;
	header = (BmpHeader*)_header;

	if ( size < sizeof( BmpHeader ) )
		return false;

	if ( header->fh.bfType != 'MB' )
		return false;

	if ( header->ih.biSize != sizeof( BITMAPINFOHEADER ) )
		return false;

	if ( header->ih.biBitCount == 4 )
	{
		if ( (header->ih.biWidth % 4) != 0 )
		{
			msg = "4-bit texture " + std::string( name ) + "'s width isn't a multiple of 4";
			return false;
		}
	}
	else if ( header->ih.biBitCount == 8 )
	{
		if ( ( header->ih.biWidth % 2) != 0 )
		{
			msg = "8-bit texture " + std::string( name ) + "'s width isn't a multiple of 2";
			return false;
		}
	}
	else if ( header->ih.biBitCount == 24 )
	{
		if ( (header->ih.biWidth % 2*3) != 0 )
		{
			msg = "24-bit texture " + std::string( name ) + "'s width isn't a multiple of 2";
			return false;
		}
	}

	_width = header->ih.biWidth;
	_bufWidth = header->ih.biWidth;
	_bitdepth = header->ih.biBitCount;
	_cbRow = header->ih.biWidth * ( _bitdepth==8 ? 8 : 16) / 8;
	_bufHeight = _height = header->ih.biHeight;

	if ( _bitdepth != 4 && _bitdepth != 8 && _bitdepth != 24 )
	{
		msg = std::string( name ) + " isn't a 4, 8, or 24-bit BMP file";
		return false;
	}

	return true;
}


void
WindowsBitmap::ReadBitmap( std::istream& input )
{
	_CMapStart = 0;
	_CMapLength = _header.ih.biClrUsed ? _header.ih.biClrUsed : (1<<_bitdepth);

	pixels = (void*)malloc( _bufHeight * _cbRow );
	assert( pixels );

	if ( bitdepth() == 4 || bitdepth() == 8 )
	{
		palette = (RGBQUAD*)alloca( _CMapLength * sizeof( RGBQUAD ) );
		assert( palette );
		input.read( (char*)palette, _CMapLength * sizeof( RGBQUAD ) );
		for ( int i=0; i<_CMapLength; ++i )
			assert( palette[i].rgbReserved == 0 );

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

	input.seekg( _header.fh.bfOffBits );

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
					*pixeldata++ = BR_COLOUR_BGRA( r, g, b, 0 );
				}
			}
			_bitdepth = 16;
			_calculatePaletteInfo( NULL, 0 );
			break;
		}
	}
}


WindowsBitmap::WindowsBitmap( std::istream& input, const char* szBitmapName ) : Bitmap()
{
	name = strdup( szBitmapName );
	assert( name );

	input.read( (char*)&_header, sizeof( _header ) );

	std::string msg;
	if ( !Validate( &_header, sizeof( _header ), msg ) )
	{
		std::cerr << szBitmapName << " is not a BMP format" << std::endl;
		//std::cerr << msg;	// << std::endl;
		return;
	}

	ReadBitmap( input );
	assert( bitdepth() == 4 || bitdepth() == 8 || bitdepth() == 16 );

	if ( bitdepth() == 16 )
	{
		foreach_pixel( Pixel_TransparentRemap );
#if 0
		if ( ts == TARGET_PLAYSTATION )
			foreach_pixel( Pixel_RGB_BRG );
#endif
	}

	loadColourCycles();

#if 0
	if ( bitdepth() == 16 && bForceTranslucent )
	{
		foreach_pixel( Pixel_ForceTranslucent );
		foreach_pixel( Pixel_VerifyTranslucent );
	}
#endif
}


void
WindowsBitmap::save( std::ostream& output ) const
{
#pragma message( "WindowsBitmap::save() not completed" )
	BmpHeader bmh;
	bmh.fh.bfType = 'MB';
	bmh.fh.bfSize = sizeof( bmh ) + _width * _height * 3;
	bmh.fh.bfReserved1 = 0;
	bmh.fh.bfReserved2 = 0;
	bmh.fh.bfOffBits = sizeof( bmh );

	bmh.ih.biSize = sizeof( bmh.ih );
	bmh.ih.biWidth = _width;
	bmh.ih.biHeight = _height;
	bmh.ih.biPlanes = 1;
	assert( _bitdepth == 16 );
	bmh.ih.biBitCount = 24;
	bmh.ih.biCompression = 0;
	bmh.ih.biSizeImage = 0;
	bmh.ih.biXPelsPerMeter = 0;
	bmh.ih.biYPelsPerMeter = 0;
	bmh.ih.biClrUsed = _CMapLength;
	bmh.ih.biClrImportant = _CMapLength;

	output.write( (char*)&bmh, sizeof( bmh ) );

	uint8* pixelLineData = (uint8*)alloca( _width * 3 );
	assert( pixelLineData );
	for ( int y = _height-1; y >= 0; --y )
	{
		uint16* pixeldata16 = (uint16*)pixels + (y * _bufWidth);
		for ( int x=0; x<_width; ++x )
		{
			pixelLineData[ x*3 + 0 ] = BR_RED_S( pixeldata16 );
			pixelLineData[ x*3 + 1 ] = BR_GRN_S( pixeldata16 );
			pixelLineData[ x*3 + 2 ] = BR_BLU_S( pixeldata16 );
			++pixeldata16;
		}
		output.write( (char*)pixelLineData, _width * 3 );
	}
}

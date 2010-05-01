// tga.cc
//
// by William B. Norris IV
// Copyright 1995 Cave Logic Studios, Ltd.  
//
// 08 Dec 95	WBNIV	Targa-specific instance of Bitmap class created
// 18 Jan 96	WBNIV	Swap color values around for Playstation version (RGB->BRG)
// 16 Feb 96	WBNIV   Prints enumeration list
// 07 Jul 96	WBNIV	Alpha channel support
// 10 Apr 97	WBNIV	Allows user to specify the transparent colour

#include <cassert>
#include <malloc.h>
//#include <string>
//using namespace std;

#include <gfxfmt/types.h>
#include <gfxfmt/bitmap.hp>
#include <gfxfmt/tga.hp>
extern bool bVerbose;
extern bool bDebug;

extern uint16 colTransparent;

TargaBitmap::~TargaBitmap()
{
}


TargaBitmap::TargaBitmap( const int width, const int height,
	const uint16 bgColor ) : Bitmap( width, height, bgColor )
{
}


TargaBitmap::TargaBitmap( const Bitmap& bm ) : Bitmap( bm )
{
}


#if 0
	for ( uint16* pPixel = pixeldata; pPixel < pixeldata + _cbRow/2; ++pPixel )
	{
			assert( (*pPixel & TRANSLUCENT) == 0 );
			std::cout << std::hex << *pPixel << ' ';
	}
#endif


bool
TargaBitmap::Validate( void* header, size_t size, std::string msg )
{
	if ( size < sizeof( TGA_HEADER ) )
		return false;

	TGA_HEADER* th = (TGA_HEADER*)header;

	if ( th->ImageType != 1 && th->ImageType != 2 )
	{
		msg = std::string( name ) + " isn't an ImageType I understand [1 or 2]";
		return false;
	}
	if ( th->PixelDepth != 8 && th->PixelDepth != 16 && th->PixelDepth != 24 && th->PixelDepth != 32 )
	{
		msg = std::string( name ) + " isn't an 8, 16, 24, or 32-bit Targa file";
		return false;
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
		msg = "8-bit texture " + std::string( name ) + "'s width isn't a multiple of two. (width is " + /*std::string( th->Width ) + */ ")";
		return false;
	}

//?	assert( th->IDLength == 0 );

	return true;
}


void
TargaBitmap::ReadBitmap( std::istream& input )
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
				input.read( (char*)&palette[pal].rgbRed, 1 );
				input.read( (char*)&palette[pal].rgbGreen, 1 );
				input.read( (char*)&palette[pal].rgbBlue, 1 );
				palette[pal].rgbReserved = 0;
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

		case 32:
		{
			_convertedPalette = NULL;
			for ( int y = _bufHeight-1; y >= 0; --y )
			{
				int32* pInputLine = (int32*)_alloca( _width * 4 );
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
			std::cerr << "Internal: bit depth " << _bitdepth << " not coded" << std::endl;
			exit( 1 );
		}
	}
}

std::string msgError;

TargaBitmap::TargaBitmap( std::istream& input, const char* szBitmapName ) : Bitmap()
{
	name = strdup( szBitmapName );
	assert( name );

	input.read( (char*)&th, sizeof( th ) );
	if ( !Validate( &th, sizeof( th ), msgError ) )
	{
		std::cerr << msgError.c_str() << std::endl;
		std::cerr << szBitmapName << " is not in Targa [.tga] format" << std::endl;
		return;
	}

	input.seekg( th.IDLength, std::ios::cur );

	bDebug && std::cout << "filename: " << szBitmapName << std::endl;
	ReadBitmap( input );

#if 0
	if ( bitdepth() == 16 )
	{
		foreach_pixel( Pixel_TransparentRemap );
		if ( ts == TARGET_PLAYSTATION )
			foreach_pixel( Pixel_RGB_BRG );
	}

	if ( bPowerOf2Size )
       checkAcceptablePowerof2();
#endif

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
TargaBitmap::save( std::ostream& s ) const
{
	static TGA_HEADER th =
	{
        0,	// char IDLength;
        0,	// char ColorMapType;
        2,	// char ImageType;
		0,	// int16 CMapStart;
		0,	// int16 CMapLength;
        0,	// char CMapDepth;
        0,	// int16 XOffset;
        0,	// int16 YOffset;
        0,	// int16 Width;			<<<
        0,	// int16 Height;		<<<
        0,	// char PixelDepth;		<<<
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

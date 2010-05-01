// bitmap.cc
//
// by William B. Norris IV
// Copyright 1995 Cave Logic Studios, Ltd.  
//
// ?? ??? 95	WBNIV	Original version
// 06 Dec 95	WBNIV	Added 8-bit Targa support
// 07 Dec 95	WBNIV	Paletted image support
// 10 Apr 97	WBNIV	Allows user to specify the transparent colour
// 20 Jun 97	WBNIV	Added force translucent

#include <windows.h>

#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdio>

#include <gfxfmt/types.h>
#include <gfxfmt/bitmap.hp>

extern bool bVerbose;
extern uint16 colTransparent;
extern char szColourCycle[];


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

	_calculateArea();
}


Bitmap::Bitmap( const Bitmap& bm )
{
	*this = bm;

	int cbPixels = bm._width * bm._height * sizeof( uint16 );
	pixels = malloc( cbPixels );
	assert( pixels );
	memcpy( pixels, bm.pixels, cbPixels );

#if 0
// was:	_bufWidth = _width = bm.coloursUsed();
	_bufWidth = _width = bm._width;
	_cbRow = _bufWidth * sizeof( uint16 );
	_bufHeight = _height = bm._height;
	_bitdepth = 16;

	_convertedPalette = NULL;
	_CMapStart = 0;
	_CMapLength = 0;
	_startColour = _endColour = -1;
#endif

	name = strdup( bm.name );
	assert( name );
//	_bHasTransparentColour = checkForTransparentColour();
	_subx = _suby = -1;
	_area = -1;
	xPal = yPal = -1;
	_idxPal = -1;

	_nColourCycles = bm._nColourCycles;
	assert( _nColourCycles <= _MAX_COLOUR_CYCLES );
	for ( int i=0; i<_nColourCycles; ++i )
		_colourCycle[i] = bm._colourCycle[i];
}


// swap R & B components of a 5:5:5 pixel
void
Bitmap::Pixel_RGB_BRG( uint16& pixel )
{
	uint16 r, g, b;

	r = BR_BLU_S( pixel );
	g = BR_GRN_S( pixel );
	b = BR_RED_S( pixel );

	pixel = BR_COLOUR_BGR( r, g, b );
}


void
Bitmap::Pixel_Print( uint16& colour )
{
	std::cout << colour << ' ';
}


void
Bitmap::Pixel_NOP( uint16& )
{
}


void
Bitmap::Pixel_TransparentRemap( uint16& colour )
{
	//assert( colTransparent );
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


#if !defined( NDEBUG )
void
Bitmap::Pixel_VerifyTranslucent( uint16& colour )
{
	assert( ( colour == 0 ) || ( colour & TRANSLUCENT ) );
}
#endif

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
Bitmap::Validate( void* header, size_t, std::string msg )
{
	return true;
}


void
Bitmap::ReadBitmap( std::istream& input )
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
#if 0
	assert( _nColourCycles == 0 );
	//std::cout << "loadColourCycles() name=[" << name << ']' << std::endl;
	if ( !name )
		return;

	int _nCycles = 0;
	_nCycles = GetPrivateProfileInt( name, "nCycles", _nCycles, szColourCycle );
	//std::cout << "_nCycles = " << _nCycles << std::endl;
	for ( int i=0; i<_nCycles; ++i )
	{
		char szSection[ _MAX_PATH ];
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
			std::cout << "Incomplete colour cycle specification for " << szSection << std::endl;
			return;
			}

		_colourCycle[ _nColourCycles++ ] = ColourCycle( _start, _end, _speed );
		assert( _nColourCycles <= _MAX_COLOUR_CYCLES );
	}
#endif
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
			for ( i=nEntries-1; i >= 0; --i )
			{
				if ( _histogram[ i ] && _endColour == -1 )
					_endColour = i;

				if ( _histogram[ i ] )
					_startColour = i;
			}
		}

		_CMapLength = _endColour+1;

//		*log << name << " [" << _startColour << ".." << _endColour << ']' << std::endl;

		// convert palette to destination format
		_convertedPalette = (uint16*)malloc( _CMapLength * sizeof(uint16) );
		assert( _convertedPalette );

#if 0
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
#endif
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
		//std::cout << "x " << x << " y " << y << " width " << width << std::endl;
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


#if 0
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
		std::cout << error() << "couldn't fit " << szTextureType <<  " \"" << bmAdd->name << "\" in room \"" << szRoomName << "\"" << std::endl;
#if 0
		*log << error() << "couldn't fit " << szTextureType <<  " \"" << bmAdd->name << "\" in room \"" << szRoomName << "\"" << std::endl;
		bGlobalError = true;
#endif
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
#endif


#if 0
void
Bitmap::crop( const int width, const int height )
{
	_width = width;
	_height = height;
}
#endif

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


#define powerof2( i )	( (i==16) || (i==32) || (i==64) )

bool
Bitmap::checkAcceptablePowerof2()
{
	bool ok = true;

	assert( name );

	if ( !powerof2( width() / (16/bitdepth() ) ) )
	{
		std::cerr << /*error() << */ "Texture " << name <<
			"'s width is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  std::endl;
#if 0
		*log << /*error() << */ "Texture " << name <<
			"'s width is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  std::endl;
#endif
		ok = false;
#if 0
		bGlobalError = true;
#endif
	}

	if ( !powerof2( height() ) )
	{
		std::cerr << /*error() << */ "Texture " << name <<
			"'s height is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  std::endl;
#if 0
		*log << /*error() << */ "Texture " << name <<
			"'s height is not an acceptable power of two (" <<
			width() << 'x' <<
			height() << 'x' <<
			bitdepth() << "bit)" <<  std::endl;
#endif
		ok = false;
#if 0
		bGlobalError = true;
#endif
	}

	return ok;
}


void
Bitmap::save( const char* filename ) const
{
	std::ofstream* output = new std::ofstream( filename, std::ios::binary );
	assert( output );
	assert( output->good() );

	save( *output );

	delete( output );
}


#include <malloc.h>

// colour reduction

int
Bitmap::findPaletteEntry( uint16* palette, int nEntries, uint16 colour )
{
	for ( int i=0; i<nEntries; ++i )
	{
		if ( palette[ i ] == colour )
			return i;
	}

	return -1;
}


bool
Bitmap::addToPalette( uint16* palette, int& idxColour, uint16 colour )
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







void
Bitmap::reduceTo16()
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

	for ( y = 0; y < _bufHeight; ++y )
	{
		uint16* pixeldata16 = (uint16*)pixels + (y * _width);

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
Bitmap::reduceTo8( uint16* palette, int nColours )
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
			*pixeldata8 = findPaletteEntry( _convertedPalette, nColours, *pixeldata16 );
			assert( *pixeldata8 != -1 );
			++pixeldata8, ++pixeldata16;
		}
	}

	_bitdepth = 8;
	_cbRow = _width * _bitdepth/8;

	checkForTransparentColour();

	_calculateArea();
}


void
Bitmap::reduceTo4( uint16* palette, int nColours )
{
	assert( bitdepth() == 8 );

	_bitdepth = 4;
	_calculateArea();
}

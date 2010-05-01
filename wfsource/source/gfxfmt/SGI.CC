// sgi.cc

#include <cassert>
#include <cstdlib>
#include <malloc.h>
#include <cstring>
#include <gfxfmt/bitmap.hp>
#include <gfxfmt/sgi.hp>

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


static uint32
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


static uint16
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
SgiBitmap::Validate( void* header, size_t size, std::string msg )
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


void
SgiBitmap::ReadBitmap( std::istream& input )
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


SgiBitmap::SgiBitmap( std::istream& input, const char* szBitmapName ) : Bitmap()
{
	name = strdup( szBitmapName );
	assert( name );

	input.read( (char*)&sh, sizeof( sh ) );
	std::string msg;
	if ( !Validate( &sh, sizeof( sh ), msg ) )
	{
		std::cerr << szBitmapName << " is not an SGI format" << std::endl;
		//std::cerr << msg << std::endl;
		return;
	}

	bDebug && std::cout << "filename: " << szBitmapName << std::endl;
	ReadBitmap( input );
	_calculateArea();

#if 0
	if ( bitdepth() == 16 )
	{
		foreach_pixel( Pixel_TransparentRemap );
		if ( ts == TARGET_PLAYSTATION )
			foreach_pixel( Pixel_RGB_BRG );
	}
#endif

	loadColourCycles();

#if 0
	if ( bForceTranslucent )
	{
		foreach_pixel( Pixel_ForceTranslucent );
		foreach_pixel( Pixel_VerifyTranslucent );
	}
#endif
}


void
SgiBitmap::save( std::ostream& output ) const
{
	assert( 0 );
}

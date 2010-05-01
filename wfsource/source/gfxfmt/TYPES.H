#ifndef TYPES_H
#define TYPES_H

#include <pigsys/pigsys.hp>
typedef enum
{
	TARGET_PLAYSTATION,
	TARGET_SATURN,
	TARGET_WINDOWS,
	TARGET_DOS
} TargetSystem;

inline int round( const int n, const int align )
{
	return ( (n+align-1) / align) * align;
}


// convert R G B components into a 5:5:5 pixel
#define BR_COLOUR_BGRA(r,g,b,a) \
		(((unsigned short)(b) << 7) & 0x7c00) |\
		(((unsigned short)(g) << 2) & 0x3e0) |\
		((unsigned short)(r) >> 3) | \
		((unsigned short)(a) << 15 )

// convert R G B components into a 5:5:5 pixel
#define BR_COLOUR_BGR(r,g,b) \
		(((unsigned short)(b) << 7) & 0x7c00) |\
		(((unsigned short)(g) << 2) & 0x3e0) |\
		((unsigned short)(r) >> 3)

#define BR_RED_S(c) (((unsigned short)c << 3) & 0xff)
#define BR_GRN_S(c) (((unsigned short)c >> 2) & 0xf8)
#define BR_BLU_S(c) (((unsigned short)c >> 7) & 0xf8)


inline uint16
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


inline uint16
RGBA_555( uint32 r, uint32 g, uint32 b, uint32 a )
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

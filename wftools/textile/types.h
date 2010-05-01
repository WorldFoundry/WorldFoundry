//==============================================================================
// types.h
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

#ifndef TYPES_H
#define TYPES_H

typedef enum
{
	TARGET_PLAYSTATION,
	TARGET_SATURN,
	TARGET_WINDOWS,
	TARGET_DOS,
	TARGET_LINUX,
} TargetSystem;
      
#if 0
typedef char int8;
typedef unsigned char uint8;
typedef unsigned short int16;
typedef unsigned short uint16;
typedef unsigned long int32;
typedef unsigned long uint32;
#endif


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

#define BR_RED_S(c) (((unsigned short)c << 3) & 0xff)
#define BR_GRN_S(c) (((unsigned short)c >> 2) & 0xf8)
#define BR_BLU_S(c) (((unsigned short)c >> 7) & 0xf8)
#define BR_ALPHA_S(c) (((unsigned short)c >> 15))



inline uint16
RGBA_555( uint32 r, uint32 g, uint32 b, uint32 a )
{
	// Short-circuit easy case (no calculations required)
	if ( a > 170 )
		return 0;

	r >>= 3;
	g >>= 3;
	b >>= 3;
	a = a < 85;
	return uint16( (a<<15) | (b<<10) | (g<<5) | (r) );
}


#endif

//==========================================================================
// _cf_PSX.cc:
// Copyright (c) 1994,95,96,97,98,99 World Foundry Group  
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

// ===========================================================================
// Original Author: Andre Burgoyne
// ===========================================================================

#define	__CF_PSX_C
#include <pigsys/pigsys.hp>

#ifdef	__PSX__

void
_psx_init(void)
{
}

#undef strdup

#if 0
char*
strdup( const char *__string )
{
	char* retVal;
	char* mem;

	assert( __string );

	// Needed for all platforms (not just PSX), because after we
	// use this memory, we need to call sys_free(), which means
	// the memory had to be allocated by sys_malloc().
	mem = (char*)malloc(strlen(__string)+1);
	assert(mem);
	AssertMemoryAllocation(mem);
	retVal = strcpy(mem,__string);
	assert( retVal );
	return retVal;
}
#endif

//=============================================================================

int
strcmpi( const char * a, const char * b )
{
	char a_char, b_char;
	char diff;
	while( *a && *b )
	{
		// convert to lower case
		a_char = tolower( *a );
		b_char = tolower( *b );

		// compare
		diff = b_char - a_char;
		if( diff != 0 ) return diff;

		a++; b++;
	}

	// if equal up to end, but not of same length?
	a_char = tolower( *a );
	b_char = tolower( *b );

	// compare
	diff = b_char - a_char;
	return diff;
}

#endif	//defined(__PSX__)

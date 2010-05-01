//==============================================================================
// mask.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include "mask.hp"
#include <cassert>
#include <cstring>

Mask::Mask( const char* szMask, const char* szName ) :
		_szName( szName ),
		_mask( 0UL ),
		_pattern( 0UL )
	{
	//cout << szName << endl;
	//assert( strlen( szMask ) == 8*4 + 7 );

	int len = strlen( szMask );
	int nBits = 0;
	for ( int i=0; i<len; ++i, ++szMask )
		{
		assert( *szMask == '0' || *szMask == '1' || *szMask == 'x' || *szMask == ' ' );

		if ( *szMask == ' ' )
			continue;

		++nBits;

		_pattern <<= 1;
		_mask <<= 1;

		if ( *szMask == '1' )
			{
			_pattern |= 1;
			_mask |= 1;
			}

		if ( *szMask == '0' )
			_mask |= 1;
		}
	assert( nBits == 32 );
	}

Mask::~Mask()
	{
	}


bool
Mask::operator()( unsigned long data ) const
	{
	return bool( ( data & _mask ) == _pattern );
	}


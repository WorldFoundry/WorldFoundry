//==============================================================================
// strtable.cc: Copyright (c) 1997-1999, World Foundry Group  
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

#include "strtable.hp"

StringTable::StringTable()
{
	_strings = NULL;
	_cbStrings = 0;
}


StringTable::~StringTable()
{
	if ( _strings )
		free( _strings );
}


int
StringTable::add( const char* szString )
{
	if ( _strings )
	{
		int nLastPos = _cbStrings - strlen( szString );
		for ( int i=0; i<nLastPos; ++i )
		{
			//printf( "Checking [%s] at %d/%d\n", _strings + i, i, nLastPos );
			if ( strcmp( _strings + i, szString ) == 0 )
				return i;
		}
	}

	int posAdded = _cbStrings;		// add to end

	_strings = (char*)realloc( _strings, _cbStrings + strlen( szString ) + 1 );
	assert( _strings );

	strcpy( _strings + _cbStrings, szString );
	_cbStrings += strlen( szString ) + 1;

	return posAdded;
}

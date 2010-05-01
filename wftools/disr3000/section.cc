//==============================================================================
// section.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include "section.hp"
#include <iostream>
using namespace std;
#include <cassert>

#if 0
bool
Section::operator==( const Section& sym ) const
{
	return _value == sym._value;
}
#endif

Section::Section( string* szSectionName, int value )
{
	_szSymbolName = szSectionName;
	_address = 0UL;
	_value = value;
	_base = 0UL;
	_data = NULL;
}

Section::Section( char* & data, int value )
{
	int cbStr = *data++;
	char* s = (char*)malloc( cbStr + 1 );
	strncpy( s, data, cbStr );
	*( s + cbStr ) = '\0';
	data += cbStr;
	string* str = new string( s, cbStr );
	_szSymbolName = str;

	_address = 0UL;
	_base = 0UL;
	_value = value;
	_alignment = 0;
	_data = NULL;
}

Section::Section()
{
	assert( 0 );
	_szSymbolName = NULL;
	_address = 0UL;
	_value = -1;
	_alignment = 0;
	_data = NULL;
}

Section::~Section()
{
	if ( _data )
		free( _data );
}


unsigned long
Section::base() const
{
	return _base;
}


int
Section::alignment() const
{
	return _alignment;
}


void
Section::alignment( int a )
{
	_alignment = a;
}


void
Section::align()
{
	assert( _alignment != 0 );
	cout << "Aligning address " << address() << " to ";
	_address = ((_address + (_alignment-1)) / _alignment) * _alignment;
	cout << address() << endl;
	assert( ( _address % _alignment ) == 0 );
}

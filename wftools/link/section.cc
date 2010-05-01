//==============================================================================
// section.cc
// Copyright (c) 1998-1999, World Foundry Group  
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
#include <pigsys/assert.hp>
#include <iostream>

#if 0
bool
Section::operator==( const Section& sym ) const
{
	return _value == sym._value;
}
#endif

Section&
Section::operator=( const Section& section )
{
	_base = section._base;
	_alignment = section._alignment;

	return *this;
}


#if 0
Section::Section( const Section& section )
{
	*this = section;
}
#endif


Section::Section( string szSectionName, int value ) : Symbol( szSectionName, value )
{
	_base = 0UL;
	_alignment = 0;
}

Section::Section( char* & data, int value ) : Symbol( string( "" ), value )
{
	int cbStr = *data++;
	char* s = (char*)malloc( cbStr + 1 );
	strncpy( s, data, cbStr );
	*( s + cbStr ) = '\0';
	data += cbStr;
	string* str = new string( s, cbStr );
	_szSymbolName = *str;

	_base = 0UL;
	_alignment = 0;
}

Section::Section() : Symbol()
{
	assert( 0 );
	_value = 0;
	_alignment = 0;
}

Section::~Section()
{
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

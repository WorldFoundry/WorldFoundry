//==============================================================================
// fn.cc
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

#include "fn.hp"
#include <pigsys/assert.hp>

bool
Function::operator==( const Function& fn ) const
{
	return //_offset == fn._offset
	_section == fn._section
	&& _value == fn._value;
}

Function::Function( string str, int value, int section, int offset, bool bDefined ) : Symbol( str, value )
{
	_section = section;
	_offset = offset;
	_bDefined = bDefined;
}


Function::Function( char* & data, int value, int section, int offset, bool bDefined ) : Symbol( data, value )
{
	_section = section;
	_offset = offset;
	_bDefined = bDefined;
}


Function::~Function()
{
}


Function::Function() : Symbol()
{
	assert( 0 );
	_section = 0;
	_offset = -1;
}


int
Function::section() const
{
	return _section;
}


int
Function::offset() const
{
	return _offset;
}


void
Function::offset( int offset )
{
	_offset = offset;
	_bDefined = true;
}


bool
Function::defined() const
{
	return _bDefined;
}


unsigned long
Function::address() const
{
	return _address;
}


void
Function::address( unsigned long address )
{
	assert( 0 );
	_address = address;
}


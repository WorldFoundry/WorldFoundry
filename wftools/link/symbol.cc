//==============================================================================
// symbol.cc
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

#include "symbol.hp"
#include <pigsys/assert.hp>

////////////////////////////////////////////////////////////////////////////////

bool
Symbol::operator==( const Symbol& sym ) const
{
	return _value == sym._value;
}


Symbol&
Symbol::operator=( const Symbol& sym )
{
	_szSymbolName = sym._szSymbolName;
	_address = sym._address;
	_value = sym._value;

	return *this;
}

Symbol::Symbol( string szSymbolName, int value )
{
	_szSymbolName = szSymbolName;
	_address = 0UL;
	_value = value;
}


Symbol::Symbol( char* & data, int value )
{
	int cbStr = *data++;
	char* s = (char*)malloc( cbStr + 1 );
	strncpy( s, data, cbStr );
	*( s + cbStr ) = '\0';
	data += cbStr;
	string* str = new string( s, cbStr );
	_szSymbolName = *str;

	_address = 0UL;
	_value = value;
}


Symbol::Symbol()
{
//	assert( 0 );

	_szSymbolName = string( "" );
	_address = 0UL;
	_value = -1;
}


Symbol::~Symbol()
{
}

////////////////////////////////////////////////////////////////////////////////

const string&
Symbol::name() const
{
	return _szSymbolName;
}


int
Symbol::value() const
{
	return _value;
}


unsigned long
Symbol::address() const
{
	return _address;
}


void
Symbol::address( unsigned long address )
{
	_address = address;
}

////////////////////////////////////////////////////////////////////////////////

SymbolByName::SymbolByName( const Symbol& sym ) : _sym( sym )
{
}


bool
SymbolByName::operator()( const Symbol& sym )
{
	return _sym.name() == sym.name();
}

////////////////////////////////////////////////////////////////////////////////

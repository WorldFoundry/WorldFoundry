//==============================================================================
// code.cc
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

#include "code.hp"
#include <pigsys/assert.hp>
#include <stdlib.h>
#include <string.h>

Code&
Code::operator=( const Code& code )
{
	_offset = code.offset();
	_size = code.size();
	_data = code.data();
	_nSection = code.section();
//	_data = (char*)malloc( _size );
//	assert( _data );
//	memcpy( _data, code.data(), _size );

	return *this;
}


Code::Code( const Code& code )
{
	*this = code;
}


Code::Code( const char* data, int offset, int size, int nSection )
{
	_offset = offset;
	assert( size > 0 );
	_size = size;
	assert( data );
	_data = data;
	_nSection = nSection;
//	_data = (char*)malloc( size );
//	assert( _data );
//	memcpy( _data, data, size );
}


Code::Code()
{
	assert( 0 );
}


Code::~Code()
{
	assert( _data );
	//free( (void*)_data );
}


const char*
Code::data() const
{
	return _data;
}


int
Code::offset() const
{
	return _offset;
}


void
Code::offset( unsigned long offset )
{
	_offset = offset;
}


int
Code::size() const
{
	return _size;
}


int
Code::section() const
{
	return _nSection;
}

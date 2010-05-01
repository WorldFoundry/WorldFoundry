//==============================================================================
// range.cc
// by William B. Norris IV
// Copyright 1998,1999,2000,2003 World Foundry Group.  
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

#include <cpplib/range.hp>
#include <pigsys/assert.hp>

////////////////////////////////////////////////////////////////////////////////

int
Range_Wrap::operator++()
{
	Validate();

	++_val;
	if ( _val > _max )
	{
		assert( _val == _max+1 );
		_val = 0;
	}

	Validate();

	return _val;
}


int
Range_Wrap::operator--()
{
	Validate();

	--_val;
	if ( _val < _min )
	{
		assert( _val == _min-1 );
		_val = _max;
	}

	Validate();

	return _val;
}

////////////////////////////////////////////////////////////////////////////////

int
Range_NoWrap::operator++()
{
	Validate();

	++_val;
	if ( _val > _max )
	{
		assert( _val == _max+1 );
		--_val;
	}

	Validate();

	return _val;
}


int
Range_NoWrap::operator--()
{
	Validate();

	--_val;
	if ( _val < _min )
	{
		assert( _val == _min-1 );
		++_val;
	}

	Validate();

	return _val;
}

////////////////////////////////////////////////////////////////////////////////

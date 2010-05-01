//==============================================================================
// patch.cc: Copyright (c) 1996-1999, World Foundry Group  
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

#include "patch.hp"
#include <iostream>
#include <algorithm>
using namespace std;
#include <cassert>
#include <cstdio>
#include "fn.hp"
#include "dumpobj.hp"

Patch::Patch()
{
	assert( 0 );
	_data = NULL;
	_section = -1;
	_offset = 0;
	_type = 0;
	_sectionData = NULL;
	_pc = 0UL;
}


Patch::Patch( int offset, char* data, char type, char* sectionData, int section )
{
	assert( data );
	_data = data;
	_section = section;
	_offset = offset;
	_type = type;
	_sectionData = sectionData;
	_pc = sections[ section ].address();
}


Patch::~Patch()
{
}


void
Patch::apply()
{
	if ( bPrintAll )
	{
		cout << "Apply patch (type) " << dec << _type << " at offset " <<
			hex << _offset << " at base of " << long( _sectionData ) << endl;
	}

	unsigned long* addr;
	unsigned long data;

	evaluate( 0UL, addr, data );
	if ( addr )		// temp
	{
		if ( *addr == 0x08000000 /* J */ || *addr == 0x0C000000 /* JAL */ )
			data >>= 2;
		bPatches = false;
		if ( bPatches )
			*addr |= data;
	}
}


int
Patch::section() const
{
	return _section;
}


bool
Patch::operator==( const Patch& patch ) const
{
	return ( _offset == patch._offset ) && ( _section == patch._section );
}


void
Patch::evaluate( unsigned long pc, unsigned long* & addr, unsigned long& data )
{
	unsigned long ret = evaluate();

	addr = NULL;

	switch ( _type )
	{
		case 30:		// untested
			ret <<= 2;
			ret += _pc;
			data = ret;
			addr = (unsigned long*)_data;
			break;

		case 82:	// should be absolute offset instead of ???
			data = (ret >> 16) & 0xFFFF;
			addr = (unsigned long*)( _sectionData + _offset );
			break;

		case 84:	// should be absolute offset instead of ???
			data = ret & 0xFFFF;
			addr = (unsigned long*)( _sectionData + _offset );
			break;

		case 16:		// data patch -- don't do anything for disassembler
			break;

//		case 72:

		case 74:
			data = ret;
			addr = (unsigned long*)( _sectionData + _offset );
			break;

		default:
			cerr << "Unknown patch type of " << int( _type ) << endl;
			assert( 0 );
			break;
	}
}


unsigned long
Patch::evaluate()
{
	assert( _data );
	unsigned long ret;
	int t1, t2;
	int op;
	long int i1;

	op = *_data++;
	switch( op )
	{
		case 0:		// immediate
			i1 = readl( _data );
			return i1;

		case 2:		// symbol
		{
			i1 = readw( _data );
			return 0x80000 | i1;
		}

		case 12:	// sectstart()
			i1 = readw( _data );
			//assert( 0 );
			_section = i1;
			return sections[ _section ].address();

		case 4:		// sectbase()
			i1 = readw( _data );
			_section = i1;
			return sections[ _section ].base();

		case 6:		// bank()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 8:		// sectof()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 10:	// offs()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 14:	// groupstart()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 16:	// groupof()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 18:	// seg()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 20:	// grouporg()
			assert( 0 );
			i1 = readw(_data);
			return 0;

		case 22:	// sectend()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		case 24:	// groupend()
			assert( 0 );
			i1 = readw( _data );
			return 0;

		default:
		{
			unsigned long t1 = evaluate();
			unsigned long t2 = evaluate();
			switch( op )
			{
				case 44 :
					ret = t1 + t2;
					break;

				case 46 :
					ret = t1 - t2;
					//assert( ret == 12 );
					break;

				default:
					cerr << "Unknown operation of " << op << endl;
					assert( 0 );
					break;
#if 0
				case 32 : bPrint && printf("="); break;
				case 34 : bPrint && printf("<>"); break;
				case 36 : bPrint && printf("<="); break;
				case 38 : bPrint && printf("<"); break;
				case 40 : bPrint && printf(">="); break;
				case 42 : bPrint && printf(">"); break;
				case 48 : bPrint && printf("*"); break;
				case 50 : bPrint && printf("/"); break;
				case 52 : bPrint && printf("&"); break;
				case 54 : bPrint && printf("!"); break;
				case 56 : bPrint && printf("^"); break;
				case 58 : bPrint && printf("<<"); break;
				case 60 : bPrint && printf(">>"); break;
				case 62 : bPrint && printf("%"); break;
				case 64 : bPrint && printf("---"); break;
				case 66 : bPrint && printf("-revword-"); break;
				case 68 : bPrint && printf("-check0-"); break;
				case 70 : bPrint && printf("-check1-"); break;
				case 72 : bPrint && printf("-bitrange-"); break;
				case 74 : bPrint && printf("-arshift_chk-"); break;
				default : bPrint && printf("?%d?", op); break;
#endif
			}
		}
	}
	return ret;
}

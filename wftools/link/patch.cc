//==============================================================================
// patch.cc
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

#include "patch.hp"
#include <pigsys/assert.hp>
#include <stdio.h>
#include <iostream>
#include <stl.h>
#include "fn.hp"
#include "dumpobj.hp"
#include "objfile.hp"

class ObjectFile;
extern vector<Function> lstGlobals;

Patch::Patch()
{
	assert( 0 );
}


Patch::Patch( int offset, char* data, char type, char* sectionData, ObjectFile* of, int section )
{
	assert( data );
	_data = data;
	_section = section;
	_offset = offset;
	_type = type;
	_sectionData = sectionData;
	assert( of );
	_of = of;
	_pc = _of->sections[ section ].address();
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

	bool bFound = evaluate( 0UL, addr, data );
	assert( bFound );
	if ( addr )		// temp
	{
		if ( *addr == 0x08000000 )	// J
			data >>= 2;
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


bool
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
			assert( 0 );
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

	return true;
}


unsigned long
Patch::evaluate()
{
	assert( _data );
	unsigned long ret;
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

			char* sz = "\01_";
			Function functionToFind( sz, i1, _of->currentSection, -1 );
			const Function* fn = find( _of->functions.begin(), _of->functions.end(), functionToFind );
			if ( fn == _of->functions.end() )
			{	// Not found in file scope -- check globals
				fn = find( lstGlobals.begin(), lstGlobals.end(), functionToFind );
				assert( fn != lstGlobals.end() );
			}
			assert( fn->defined() );
			return fn->offset();	// + _of->sections[ fn->section() ]->
			//return 0;
		}

		case 12:	// sectstart()
			i1 = readw( _data );
			_section = i1;
			return _of->sections[ i1 ].address();

		case 4:		// sectbase()
			i1 = readw( _data );
			_section = i1;
			return _of->sections[ i1 ].base();

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
		{
			i1 = readw( _data );
			Section& section = _of->sections[ i1 ];
			return section.address() + 1000;	// size
		}

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

				case 50:
					ret = t1 / t2;
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

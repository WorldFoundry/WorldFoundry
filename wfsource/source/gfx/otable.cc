//=============================================================================
// otable.cc: ordering table class
// Copyright ( c ) 1997,1998,1999,2000,2001 World Foundry Group  
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

// ===========================================================================
// Description:
//
// Original Author: Kevin T. Seghetti
//============================================================================

#include <gfx/otable.hp>
#include <streams/dbstrm.hp>
#include <memory/memory.hp>
#include <gfx/renderer.hp>


#if defined( __PSX__ )
#       pragma message ("KTS: ClearOTagR seems to clear 3 more entries than asked for")
#       define OTABLE_FUDGE 3
#else
#       define OTABLE_FUDGE 0
#endif

//============================================================================

OrderTable::OrderTable(int orderTableDepth,Memory& memory) :
	_memory(memory)
{
	_memory.Validate();
	assert(orderTableDepth > 0);
	_depth = orderTableDepth;
	_orderTable = new (_memory) ORDER_TABLE_ENTRY [ _depth+OTABLE_FUDGE ];
	assert(ValidPtr(_orderTable));
	Clear();
}

//============================================================================

#if 0
AddOT(unsigned long *ot0, unsigned long *ot1, int n)
{
	AddPrims(ot0, ot1, ot1+n-1);
}


void
AltClearOTagR( uint32 *ot, long size )
{
	register uint32 *entry;
	entry = ot + (size-1);
	while( size-- )
		*entry-- = (uint32)(entry-1) & 0x00FFFFFF;
	ot[0] = (uint32)&ot_terminator;
}
#endif

//============================================================================

OrderTable::~OrderTable()
{
	ValidatePtr( _orderTable );
	_memory.Free(_orderTable,sizeof(ORDER_TABLE_ENTRY)*_depth+OTABLE_FUDGE);
}

//============================================================================

#if DO_IOSTREAMS
std::ostream&
operator << ( std::ostream& s, const OrderTable& ot )
{
	ot.Validate();
	s << "Order Table: depth = " << ot._depth << std::endl;
	s << "Order Table array dump:" << std::endl;
	s << "idx, entry addr, entry, * entry" << std::endl;
	for(int index=ot._depth-1;index >= 0; index--)
	{
		s << index << ": (&" << &ot._orderTable[index] << ") " << std::hex << getaddr( &ot._orderTable[index] );
		if ( isendprim( &ot._orderTable[index] ) )
			s << ", [endOfPrimitives]";
		else
			s << ", *" << std::hex << *((uint32*)(ot._orderTable[index].addr));
		s << std::endl;
	}

#if 1
	s << "Order Table link dump:" << std::endl;
	Primitive* _orderTable = (Primitive*)&ot._orderTable[ot._depth-1];
	for ( ; !isendprim( _orderTable ); _orderTable = (Primitive*)nextPrim( _orderTable ) )
	{
		ValidatePtr(_orderTable);

		s << "ptr = " << _orderTable << ", *ptr = " << (void*)(*(long*)_orderTable);
#if 0
		s << "ptr->addr = " << _orderTable->dummy.addr;
		s << ", ptr->code = " << int(_orderTable->dummy.code);
#endif
		s << ", *ptr = " << (void*)(*(long*)_orderTable);
		s << ", *ptr+1 = " << (void*)(*((long*)_orderTable+1)) << ", addr = " << (void*)(getaddr(_orderTable));
#if defined( __PSX__ )
		s << ", len = " << int(getlen(_orderTable));
#endif
		s << std::endl;
	}
#endif
	return s;
}
#endif

//============================================================================

#if defined(__PSX__)
#if 0
uint32 myot_terminator = 0x04ffffff;

void
AltClearOTagR( uint32 *ot, long size )
{
	RangeCheckExclusive(0,size,10000);	// kts arbitrary
	register uint32 *entry;
	entry = ot + (size-1);
	while( size-- )
		*entry-- = (uint32)(entry-1) & 0x00FFFFFF;
	ot[0] = (uint32*)(((uint32)(&myot_terminator)) & 0xffffff);
}
#endif
#endif		// defined(__PSX__)

//==============================================================================

#if !defined( __PSX__ )

static ORDER_TABLE_ENTRY ot_terminator = { (ORDER_TABLE_ENTRY*)~0, 0, 0, 0, CODE_NOP };

void
ClearOTagR( ORDER_TABLE_ENTRY* _orderTable, int _depth )
{
	ORDER_TABLE_ENTRY* pPrimitive;
	for ( pPrimitive = _orderTable + _depth - 1; pPrimitive != _orderTable; --pPrimitive )
	{
		setaddr( pPrimitive, pPrimitive-1 );
		pPrimitive->code = CODE_NOP;
	}
	assert( pPrimitive == _orderTable );
	setaddr( pPrimitive, &ot_terminator );
	pPrimitive->code = CODE_NOP;
}

#endif			                        // !defined(__PSX__)
//============================================================================

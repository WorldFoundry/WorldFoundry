//=============================================================================
// gfx\psx\viewport.cc: viewport hardware abstraction class, psx specific cod3e
// Copyright ( c ) 1997,1998,1999,2000 World Foundry Group  
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
// Description: The viewport class encapsulates data and behavior for a single
//	 hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#include <hal/hal.h>

//==============================================================================

ViewPort::ViewPort(Display& display, int orderTableDepth, Scalar xSize, Scalar ySize, Memory& memory, int subdividePrimitivesCount) :
	_display(display)
	,_memory(memory)
	,_xSize(xSize)
	,_halfXSize( _xSize / 2 )
	,_ySize(ySize)
	,_halfYSize( _ySize / 2 )
#if defined(USE_ORDER_TABLES)
	,_primitives((LMalloc*)&__primitives)
#endif
{
	_memory.Validate();
#if defined(USE_ORDER_TABLES)
	for(int index=0;index<ORDER_TABLES;index++)
	{
		new ((void*)&_primitives[index]) LMalloc(HALLmalloc,sizeof(Primitive)*subdividePrimitivesCount 	MEMORY_NAMED( COMMA "ViewPort Primitives" ) 	);
		_primitives[index].Validate();
	}

	RangeCheck(0,orderTableDepth,10000);  // kts arbitrary
	_orderTableDepth = orderTableDepth;

	for(int otIndex=0;otIndex<ORDER_TABLES;otIndex++)
	{
		_orderTable[otIndex] = new (_memory) OrderTable(orderTableDepth,_memory);
		assert(ValidPtr(_orderTable[otIndex]));
	}
#endif
	Validate();
}

//============================================================================

ViewPort::~ViewPort()
{
#if defined(USE_ORDER_TABLES)
	for(int index=ORDER_TABLES-1;index>=0;index--)
		MEMORY_DELETE(_memory,_orderTable[index],OrderTable);
#endif
}

//============================================================================

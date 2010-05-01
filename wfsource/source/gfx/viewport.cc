//=============================================================================
// viewport.cc: viewport hardware abstraction class
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
// Description: The viewport class encapsulates data and behavior for a single
//	 hardware screen
// Original Author: Kevin T. Seghetti
//============================================================================

#include <gfx/viewport.hp>

#if defined ( RENDERER_PSX )
#include <gfx/psx/viewport.cc>
#elif defined ( RENDERER_GL )
#include <gfx/gl/viewport.cc>
#elif defined ( RENDERER_XWINDOWS )
#include <gfx/xwindows/viewport.cc>
#elif defined ( RENDERER_DIRECTX )
#include <gfx/directx/viewport.cc>
#else
#error no viewport code for this platform!
#endif

//============================================================================

#if defined(USE_ORDER_TABLES)
Primitive*
ViewPort::DuplicatePrimitive(const Primitive& original, int count)
{

	Primitive* prims = (Primitive*)_primitives[_display.GetConstructionOrderTableIndex()].Allocate(count*sizeof(Primitive) ASSERTIONS( COMMA __FILE__ COMMA __LINE__ ));
	assert(ValidPtr(prims));
	if(prims)
		for (int index=0;index<count;index++ )
		{
			// kts note: this could cause a read from invalid memory
			memcpy((void*)&prims[index],(void*)&original,sizeof(Primitive));
		}
	return(prims);
}
#endif

//============================================================================

//=============================================================================
// new.cc.c:
// Copyright (c) 1997,99 World Foundry Group  
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
// Description: The beginnings of the new memory managnemt system
// Original Author: Kevin Seghetti
//=============================================================================

#define	_NEW_CC
#include <pigsys/pigsys.hp>
#include <pigsys/_atexit.h>

#if 0
//=============================================================================

void * operator new( size_t _sz, char * file, int line )
{
	(void)file;
	(void)line;
	assert(0);					        // kts must use Memory system
//	void* memory = malloc(_sz);
//	printf("new: size = %d, from %s:%d at address %x\n",_sz, file, line, memory);
//	return memory;
	return NULL;
}

//=============================================================================

void * operator new [] ( size_t _sz, char * file, int line )
{
	(void)file;
	(void)line;
	assert(0);				            // kts must use Memory system
//	void* memory = malloc(_sz);
//	printf("new: size = %d, from %s:%d at address %x\n",_sz, file, line, memory);
//	return memory;
	return NULL;
}
#endif

//=============================================================================

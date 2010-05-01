//==============================================================================
// aitoken.h: Copyright (c) 1996-1999, World Foundry Group  
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

#ifndef AITOKEN_H
#define AITOKEN_H

#if defined( STL_SYMBOLS )
#   include <map>
#	include <string>
using namespace std;
#else
typedef struct
	{
	char* szSymbolName;
	int nSymbolValue;
	} Symbol;

extern int nSymbols;
extern Symbol tblSymbols[];
#endif

#endif	// AITOKEN_H

//===========================================================================*/
// _atexit.h:
// Copyright (c) 1994,95,96,97,98,99 World Foundry Group  
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
// Original Author: Andre Burgoyne
// ===========================================================================
//===========================================================================*/

#ifndef	__ATEXIT_H
#define	__ATEXIT_H

// Implementation notes:	sys_atexit is provided mostly for develpment targets
// where sometimes one has to deal with abnormal termination conditions.
// Thus the semantics of this function are slight different than the standard
// atexit function, which only executes when exit() is called.  In our case we want
// our functions executed on other conditions as well, such as user interreupts.
// Use of this is not recommended for final targets, where abnormal termination
// represents a serious bug.

#ifndef	_PIGSYS_H
#include <pigsys/pigsys.hp>
#endif	//!defined(_PIGSYS_H)

// sys_atexit is declared in pigsys.h, this is private interface only

#ifndef	SYS_MAC_ATEXIT_FUNCS
#define	SYS_MAC_ATEXIT_FUNCS	4
#endif	//!defined(SYS_MAC_ATEXIT_FUNCS)

extern C_LINKAGE void	_sys_call_atexit_funs(int code);

//===========================================================================*/
#endif	//!defined(__ATEXIT_H)
//===========================================================================*/

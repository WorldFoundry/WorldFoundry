//===========================================================================*/
// clib.h: included by pigsys.h. provide wrappers/declarations of standard c library
// Copyright ( c ) 1998,99 World Foundry Group  
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

//=============================================================================

// This section comments out direct use of std functions (running asserts).
// Unfortunately only the wrapped functions can be so detected, but that
// includes most of them.
#if		DO_ASSERTIONS && !defined(_SYS_NOCHECK_DIRECT_STD)

// stdlib.h
#undef  atoi
#define  atoi      sys_atoi
#undef  atol
#define  atol      sys_atol
#undef  strtol
#define  strtol    sys_strtol
#undef  strtoul
#define  strtoul   sys_strtoul

//#undef	atexit
//#define	atexit		sys_atexit

#endif	// DO_ASSERTIONS && !defined(_SYS_NOCHECK_DIRECT_STD)

// This section is for functions which are always implemented as our own
// function, thus we can always check them
#ifndef	_PIGSYS_C

// stdlib.h
#undef	exit
#define exit            sys_exit

#endif	// !defined(_PIGSYS_C)


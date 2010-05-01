// ===========================================================================
// Copyright (c) 1994,95,96,97,99 World Foundry Group  
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
// Description: Generic configuration file for ANSI compiler under Win32.
//============================================================================

#ifndef	_CF_WIN_H
#define	_CF_WIN_H

//============================================================================

#define	WF_BIG_ENDIAN		0
#define _BOOL_IS_DEFINED_ __cplusplus

#ifndef __WINDOWS__
#define __WINDOWS__
#endif	//!defined(__WINDOWS__)

#define PATH_MAX _MAX_PATH
#define DIRECTORY_SEPARATOR '\\'


//#if !defined( __MSDOS__ )
//#define __MSDOS__		// temporary
//#endif	// !defined( __MSDOS__ )

	// short access is efficient on this architecture
//#define	SYS_SMALLINT	short

#if defined( __WATCOMC__ )
#	if ( __WATCOMC__ < 1060 )
#		error You need to use Watcom C/C++ 10.6
#	endif
#	if !defined( __FLAT__ )
#		error Not compiling for 32-bit
#	endif
#	if ( _M_IX86 < 500 )
#		error Not compiling for Pentium
#	endif



	#if defined( __cplusplus )
   
   //	#pragma warning 14 9;	// parameter not used
   	#pragma warning 367 9;	// possibly incorrect assignment
   	#pragma warning 391 9;	// assignment found within boolean expresion
   	#pragma warning 17 9;	// label 'label' not referenced
   	#pragma warning 628 9;	// expression is not meaningful

		#pragma warning 594 9;	// don't know how to have go away in MemCheck v2.1
			// These are simply silly warnings
		#pragma warning 604 9;	// must lookahead to determine...
		#pragma warning 595 9;	// construct resolved as...
		#pragma warning 364 9;	// adjacent ==, !=, etc. operators
		#pragma warning 689 9;	// conditional expression always true
		#pragma warning 690 9;	// conditional expression always false
		#pragma warning 555 9;	// while expression always false
			// These may indicate a real problem, hence they are
			// turned on by default (one can always override CCWARN_FLAG
			// in top-level userdefs.mk if one wants).  However these
			// problems should be fixed in code, e.g.:
			//	W14:	void foo( char* /*arg_not_used*/ );
			//  W391:	if ( (fp = sys_fopen(..)) != (sys_FILE*)NULL )
			//  W367:	proper cast if needed, etc.
			// WBN: WRONG
		#pragma warning 391 9;	// assignment found within boolean expresion
		#pragma warning 367 9;	// possibly incorrect assignment
		#pragma warning  14 8;	// parameter not used
		#pragma warning   4 8;	// base class has no virtual destructor
	#else	//!defined(__cplusplus)
		#pragma disable_message (202);		// parameter not used (in C)
	#endif	//!defined(__cplusplus)
#endif	//defined(__WATCOMC__)


START_EXTERN_C
extern void			_win32_init(void);
END_EXTERN_C

// rindex doesn't exist under windows
char *rindex(char *s, int c);

//==============================================================================
// because microsoft loves to fsck everything up
#define snprintf _snprintf

//============================================================================
#endif	//!defined(_CF_WIN_H)
//============================================================================

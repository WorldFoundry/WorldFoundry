//=============================================================================
// cf_psx.h:
//=============================================================================

#ifndef	_CF_PSX_H
#define	_CF_PSX_H

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
// Description: Generic configuration file compiling for PSX.
// Original Author: Andre Burgoyne
// ===========================================================================

#define	WF_BIG_ENDIAN		0
#define _BOOL_IS_DEFINED_ 0

#ifndef __PSX__
#define __PSX__
#endif


extern "C" {
#include <libsn.h>
}

#if DBSTREAM < 1
extern bool enableScreenStream;        // kts remove in final game, used
										// to print streams to the psx screen
#endif

// GNU doesn't have this yet...
#define explicit

	// is short access is efficient on this architecture?
//#define	SYS_SMALLINT	short

#include <stddef.h>
#undef	_STDDEF_H
#define	_STDDEF_H

	// none of stdin, stdout, stderr are defined for this target
#undef	_SYS_NO_STDOUT
#define	_SYS_NO_STDOUT
#undef	_SYS_NO_STDIN
#define	_SYS_NO_STDIN
#undef	_SYS_NO_STDERR
#define	_SYS_NO_STDERR

#undef	_SYS_NO_UNBUFFERED_STDOUT
#define	_SYS_NO_UNBUFFERED_STDOUT

	// These are not defined in the header files.
START_EXTERN_C

#include <stdio.h>
#undef	_STDIO_H
#define	_STDIO_H

//	 These are required to compile pigsys library.  Functions which do
//	 not exist at all we define to cause run-time errors.
//	 Functions which are not declared, but actually exist in the library
//	 should simply work.  One can check this by linking, and seeing
//	 which functions do not exist.  See the bottom of pigsys.c for list
//	 of stubs.

	// If FILE does not get defined in stdio.h, then make a dummy decl:
//#define	sys_FILE	struct _FILE
//sys_FILE;

typedef struct
{
	int a;
	// contains nothing
} FILE;

#define sys_FILE 	FILE

#ifndef	EOF
#define	EOF			(-1)
#endif	//!defined(EOF)

#ifndef	SEEK_SET
#define	SEEK_SET	0
#endif	//!defined(SEEK_SET)

#ifndef	SEEK_CUR
#define	SEEK_CUR	1
#endif	//!defined(SEEK_CUR)

#ifndef	SEEK_END
#define	SEEK_END	2
#endif	//!defined(SEEK_END)

	// these not defined, guessing values
#ifndef	FILENAME_MAX
#define	FILENAME_MAX 256
#endif	//!defined(FILENAME_MAX)
#ifndef	FOPEN_MAX
#define	FOPEN_MAX 32
#endif	//!defined(FOPEN_MAX)
#define DIRECTORY_SEPARATOR '\\'



extern FILE*	afopen(const char* fname, const char* mode);
extern sys_FILE*	bfopen(const char* fname, const char* mode);

extern int			sscanf(const char* buf, const char* fmt, ...);
extern sys_FILE*	fopen(const char* fname, const char* mode);
extern int			fflush(sys_FILE* fp);
extern int			fclose(sys_FILE* fp);
extern int			fgetc(sys_FILE* fp);
extern char*		fgets(char* s, int n, sys_FILE* fp);
extern int			fputc(int c, sys_FILE* fp);
extern int			ungetc(int c, sys_FILE* fp);
extern size_t		fread(void* buf, size_t size, size_t nobj, sys_FILE* fp);
extern size_t		fwrite(const void* buf, size_t size, size_t nobj, sys_FILE* fp);
extern int			fseek(sys_FILE* fp, long off, int orig);
extern long			ftell(sys_FILE* fp);
extern int			feof(sys_FILE* fp);

#undef strdup
extern char* 		strdup( const char* __string );
extern void strlwr( char* string );
extern int strcmpi( const char * a, const char * b );

// Fixed with 3.1
//extern void *memcpy( void *__s1, const void *__s2, size_t __n );
//extern int   memcmp( const void *__s1, const void *__s2, size_t __n );
//extern int			printf(const char* fmt, ...);
//extern int strcmp( const char *__s1, const char *__s2 );
//extern char *strncpy( char *__s1, const char *__s2, size_t __n );
//extern char *strcat( char *__s1, const char *__s2 );
//extern char *strchr( const char *__s, int __c );
//extern char *strrchr( const char *__s, int __c );
//extern void *memmove( void *__s1, const void *__s2, size_t __n );
//extern void *memset( void *__s, int __c, size_t __n );
//extern char *strcpy( char *__s1, const char *__s2 );
//extern char *strncat( char *__s1, const char *__s2, size_t __n );
//extern int strncmp( const char *__s1, const char *__s2, size_t __n );
//extern void *memchr( const void *__s, int __c, size_t __n );
//extern size_t strlen( const char *__s );
//#include <strings.h>
//#include <memory.h>

// paw
//extern void exit( int );

END_EXTERN_C

//=============================================================================
#endif	//!defined(_CF_PSX_H)
//=============================================================================

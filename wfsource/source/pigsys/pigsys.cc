//=============================================================================
// pigsys.c:
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
// Description: The base implementation for system dependant interface.
// Original Author: Andre Burgoyne
//=============================================================================

#define	_PIGSYS_C
#define	_SYS_NOCHECK_DIRECT_STD	1
#include <pigsys/pigsys.hp>
#include <pigsys/_atexit.h>

//=============================================================================

static int _sys_exitcode;

#if !defined( __WIN__ )
	int __argc;
	char* * __argv;
#endif

#ifndef	_SYS_CAST_2CHARP
#define	_SYS_CAST_2CHARP	//nothing
#endif	//!defined(_SYS_CAST_2CHARP)

#ifndef	_SYS_CAST_2INT
#define	_SYS_CAST_2INT	//nothing
#endif	//!defined(_SYS_CAST_2INT)


#define	LOGFLAG		"-log="
#define	VERSIONFLAG	"-v"
#define QUIETFLAG	"-q"

// stdio.h

#if		DO_ASSERTIONS

// undefine all of these, this means we have use the sys_ version in this file to get protection
// since we need to call the actual routines from here
#undef fopen
#undef fflush
#undef fclose
#undef fgetc
#undef fgets
#undef fputc
#undef ungetc
#undef fread
#undef fwrite
#undef fseek
#undef ftell
#undef feof
#undef ferror
#undef rewind

#undef	strcmp
#undef	strncmp
#undef	strchr
#undef	strrchr
#undef	strlen

#undef exit

#undef atoi
#undef strtol
#undef strtoul

//=============================================================================

#define	TARG_UNIMPLEMENTED(_targ,_fun)	\
	Fail(#_fun " is not yet implemented on the " #_targ ".\n");

//=============================================================================
//	This is a list of stubs for quickly implementing incomplete ANSI
//	systems. Start with nothing, then add in any stubs which are not
//	found in the libraries.
//
//int			sscanf(const char* buf, const char* fmt, ...)
//				{ TARG_UNIMPLEMENTED(PSX,sscanf); return 0; }
//sys_FILE*	fopen(const char* fname, const char* mode)
//				{ TARG_UNIMPLEMENTED(PSX,fopen); return (sys_FILE*)NULL; }
//int			fflush(sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fflush); return EOF; }
//int			fclose(sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fclose); return EOF; }
//int			fgetc(sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fgetc); return EOF; }
//char*		fgets(char* s, int n, sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fgets); return (char*)NULL; }
//int			fputc(int c, sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fputc); return EOF; }
//int			ungetc(int c, sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,ungetc); return EOF; }
//size_t		fread(void* buf, size_t size, size_t nobj, sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fread); return 0; }
//size_t		fwrite(const void* buf, size_t size, size_t nobj, sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,fwrite); return 0; }
//int			fseek(sys_FILE* fp, long off, int orig)
//				{ TARG_UNIMPLEMENTED(PSX,fseek); return -1; }
//long		ftell(sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,ftell); return -1L; }
//int			feof(sys_FILE* fp)
//				{ TARG_UNIMPLEMENTED(PSX,feof); return -1; }
//
//=============================================================================

#ifdef	__PSX__

//int			sscanf(const char* buf, const char* fmt, ...)
//				{ TARG_UNIMPLEMENTED(PSX,sscanf); return 0; }
sys_FILE*	fopen(const char* /*fname*/, const char* /*mode*/)
				{ TARG_UNIMPLEMENTED(PSX,fopen); return (sys_FILE*)NULL; }
int			fflush(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fflush); return EOF; }
int			fclose(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fclose); return EOF; }
int			fgetc(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fgetc); return EOF; }
char*		fgets(char* /*s*/, int /*n*/, sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fgets); return (char*)NULL; }
int			fputc(int /*c*/, sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fputc); return EOF; }
int			ungetc(int /*c*/, sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,ungetc); return EOF; }
size_t		fread(void* /*buf*/, size_t /*size*/, size_t /*nobj*/, sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fread); return 0; }
size_t		fwrite(const void* /*buf*/, size_t /*size*/, size_t /*nobj*/, sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,fwrite); return 0; }
int			fseek(sys_FILE* /*fp*/, long /*off*/, int /*orig*/)
				{ TARG_UNIMPLEMENTED(PSX,fseek); return -1; }
long		ftell(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,ftell); return -1L; }
int			feof(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,feof); return -1; }
int			ferror(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,ferror); return -1; }
void		rewind(sys_FILE* /*fp*/)
				{ TARG_UNIMPLEMENTED(PSX,rewind); }

#endif	// defined(__PSX__)

//=============================================================================

sys_FILE*
sys_fopen(const char* fname, const char* mode)
{
	sys_FILE*	fp;

	assert( fname != NULL );
	assert( fname[0] != '\0' );
	assert( mode != NULL );
	assert( mode[0] != '\0' );
	fp = fopen(_SYS_CAST_2CHARP fname, _SYS_CAST_2CHARP mode);
	if ( fp != (sys_FILE*)NULL )
	{
	}
	return fp;
}

//==============================================================================

int
sys_fflush(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	return _SYS_CAST_2INT fflush(fp);
}

//==============================================================================

int
sys_fclose(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	return _SYS_CAST_2INT fclose(fp);
}

//==============================================================================

#if !defined( __PSX__ )
int
sys_fgetc(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	return _SYS_CAST_2INT fgetc(fp);
}

//==============================================================================

char*
sys_fgets(char* s, int n, sys_FILE* fp)
{
	assert( s != NULL );
	assert( n > 0 );
        assert( ValidPtr(fp) );
	return fgets(s, n, fp);
}

//==============================================================================

int
sys_fputc(int c, sys_FILE* fp)
{
	assert( (unsigned)c <= UCHAR_MAX || c == EOF );
        assert( ValidPtr(fp) );
	return _SYS_CAST_2INT fputc(c, fp);
}

//==============================================================================

int
sys_ungetc(int c, sys_FILE* fp)
{
	assert( (unsigned)c <= UCHAR_MAX || c == EOF );
        assert( ValidPtr(fp) );
	return _SYS_CAST_2INT ungetc(c, fp);
}
#endif

//==============================================================================

size_t
sys_fread(void* buf, size_t size, size_t nobj, sys_FILE* fp)
{
	assert( buf != NULL );
	assert( size > 0 );
	assert( nobj > 0 );
        assert( ValidPtr(fp) );
	return fread(buf, size, nobj, fp);
}

//==============================================================================

size_t
sys_fwrite(const void* buf, size_t size, size_t nobj, sys_FILE* fp)
{
	assert( buf != NULL );
	assert( size > 0 );
	assert( nobj > 0 );
        assert( ValidPtr(fp) );
	return fwrite(buf, size, nobj, fp);
}

//==============================================================================

int
sys_fseek(sys_FILE* fp, long off, int origin)
{
        assert( ValidPtr(fp) );
	assert( origin == SEEK_SET || origin == SEEK_CUR || origin == SEEK_END );
	return _SYS_CAST_2INT fseek(fp, off, origin);
}

//==============================================================================

long
sys_ftell(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	return ftell(fp);
}

//==============================================================================

int
sys_feof(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	return feof(fp);
}

//==============================================================================

int
sys_ferror(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	return ferror(fp);
}

//==============================================================================

void
sys_rewind(sys_FILE* fp)
{
        assert( ValidPtr(fp) );
	rewind(fp);
}

#endif	//DO_ASSERTIONS

//==============================================================================
// ctype.h (macro implementation only)

// string.h

#if		DO_ASSERTIONS

int
sys_strcmp(const char* s1, const char* s2)
{
	assert( s1 != NULL );
	assert( s2 != NULL );
	return strcmp(s1, s2);
}

//==============================================================================

int
sys_strncmp(const char* s1, const char* s2, size_t n)
{
	assert( s1 != NULL );
	assert( s2 != NULL );
	assert( n > 0 );
	return strncmp((char*)s1, (char*)s2, n);
}

//==============================================================================

char*
sys_strchr(const char* s, int c)
{
	assert( s != NULL );
	assert( (unsigned)c <= UCHAR_MAX );
	return strchr((char*)s, c);
}

//==============================================================================

char*
sys_strrchr(const char* s, int c)
{
	assert( s != NULL );
	assert( (unsigned)c <= UCHAR_MAX );
	return strrchr((char*)s, c);
}

//==============================================================================

size_t
sys_strlen(const char* s)
{
	assert( s != NULL );
	return strlen(s);
}

#endif	//DO_ASSERTIONS


//==============================================================================
// stdlib.h

#if		defined(DESIGNER_CHEATS)

void
_sys_printenv(void)
{
	printf("DEBUG = %d,\nDO_ASSERTIONS = %d, NO_ASSERT_EXPR = %d, USE_INLINE_DEFS = %d, WF_BIG_ENDIAN = %d",
		DEBUG, DO_ASSERTIONS, NO_ASSERT_EXPR, USE_INLINE_DEFS, WF_BIG_ENDIAN);
	printf("\n");
}

#endif	//defined(DESIGNER_CHEATS)

#if PIGS_USE_MEMCHECK
void
memcheck_shutdown()
{
	if(_sys_exitcode == 0)
		mc_endcheck();
}
#endif

#if defined(DESIGNER_CHEATS)
bool	bPrintVersion = 1;
#else
bool	bPrintVersion = 0;
#endif

//==============================================================================

void
_psx_init(void);

//==============================================================================

void
sys_init(int* argcp, char*** argvp)
{
	__argc = *argcp;
	__argv = *argvp;

#if PIGS_USE_MEMCHECK
    mc_startcheck( NULL );
	mc_set_alignsize( sizeof( int32 ) );
	sys_atexit( memcheck_shutdown );
#endif

#if defined( __PSX__ )
	_psx_init();
#elif defined( __WIN__ )
	_win32_init();
#elif defined(__LINUX__)
	_linux_init();
#else
#	error Unknown platform
#endif

#ifndef	_SYS_NO_STDOUT
#ifndef	_SYS_NO_UNBUFFERED_STDOUT
	assert( stdout != NULL );
//	setbuf(stdout, (char*)NULL); 		// kts removed 5/20/96 3:17PM since it causes a memory leak
#endif	//!defined(_SYS_NO_UNBUFFERED_STDOUT)
#endif	//!defined(_SYS_NO_STDOUT)

	assert( argcp != NULL );
	assert( argvp != NULL );
	if ( (*argcp) < 1 )
	{
		static char*	sargv[] = { "<noname>", (char*)NULL };

		assert( *argcp == 0 );
		(*argcp) = 1;
		(*argvp) = sargv;
	}
	assert( *argcp > 0 );

#if		defined(DESIGNER_CHEATS)
	{
		int		i;
			/// While we do recognize the command line option -log=n, we do NOT strip
			//  this out of the command line.  This is because there is no safe way
			//  of doing it short of allocating our own memory block which does not
			//  seem like it is worth the trouble.
		for ( i = 1; i < (*argcp); i++ )
		{
			if ( 0 )
				;
#if 0
		    else if ( strncmp((*argvp)[i], LOGFLAG, strlen(LOGFLAG)) == 0 )
	    	{
#if	DO_ASSERTIONS
				ulong	ll = ERR_LOG;
				sscanf((*argvp)[i]+strlen(LOGFLAG), "%li", &ll);
#else
#pragma message ("kts: fix this")
#endif
				ERR_UNSET_LOG(0xffffffffUL);
				ERR_SET_LOG(ll);
				bPrintVersion = ERR_LOG_ISSET(ERR_LOG2);
			}
#endif
			else if ( strcmp((*argvp)[i], VERSIONFLAG) == 0 )
				bPrintVersion = true;
			else if ( strcmp((*argvp)[i], QUIETFLAG) == 0 )
				bPrintVersion = false;
		}
	}
#endif	//defined(DESIGNER_CHEATS)

#if		DEBUG
	{
		uint32	l	= 0x01020304;
		uint16*	sp	= (uint16*)&l;
		uchar*	cp	= (uchar*)&l;

		if ( (WF_BIG_ENDIAN && (cp[0] != 1 || cp[1] != 2 || cp[2] != 3 || cp[3] != 4)) ||
			 (!WF_BIG_ENDIAN && (cp[0] != 4 || cp[1] != 3 || cp[2] != 2 || cp[3] != 1)) ) {
			printf("WARNING: WF_BIG_ENDIAN = %d, but byte order wrong?! (%d/%d/%d/%d)\n",
				WF_BIG_ENDIAN, (int)cp[0], (int)cp[1], (int)cp[2], (int)cp[3]);
		}
		if ( (WF_BIG_ENDIAN && (sp[0] != 0x0102 || sp[1] != 0x0304)) ||
			 (!WF_BIG_ENDIAN && (sp[0] != 0x0304 || sp[1] != 0x0102)) ) {
			printf("WARNING: WF_BIG_ENDIAN = %d, but short order wrong?! (%04x/%04x)\n",
				WF_BIG_ENDIAN, (int)sp[0], (int)sp[1]);
		}
	}
#endif	//DEBUG

#if		defined(DESIGNER_CHEATS)
	if ( bPrintVersion )
	{
		int		i;

		_sys_printenv();
		printf("argc = %d, argv: ", (*argcp));
		for ( i = 0; i < (*argcp); i++ ) {
			printf("%s ", (*argvp)[i] ? (*argvp)[i] : "<NULL>");
		}
		printf("\n");
	}
#endif	//defined(DESIGNER_CHEATS)
}

//==============================================================================

#undef exit
void
sys_exit(int retcode)
{
	_sys_exitcode = retcode;
	if ( bPrintVersion )
			printf("sys_exit: game quit\n");
	printf("sys_exit(%d) called\n", retcode);
//	ERR_DEBUG_LOG( ERR_LOG4, ("sys_exit(%d) called\n", retcode) );
#if		!_SYS_SUPPRESS_EXIT
	{
		static bool	inExit = false;

		if ( inExit )
		{
			printf("sys_exit called recursively, retcode = %d, calling exit directly.\n", retcode);
			exit(retcode);
			//NOTREACHED
		}
		inExit = true;
	}
    printf("Calling sys_call_atexit function\n");
	_sys_call_atexit_funs(retcode);
#endif	//!_SYS_SUPPRESS_EXIT
#if		_SYS_SUPPRESS_EXIT
	printf("sys_exit called, retcode = %d, exit suppressed\n", retcode);
#ifndef	_SYS_NO_STDOUT
	sys_fflush(stdout);
#endif	//!defined(_SYS_NO_STDOUT)
#else	//!_SYS_SUPPRESS_EXIT
    printf("calling exit with return code of %d\n",retcode);
#undef exit
	exit(retcode);
	//NOTREACHED
#endif	//!_SYS_SUPPRESS_EXIT
}

//==============================================================================

#if		DO_ASSERTIONS

static void
_checkDigits(const char* s, unsigned int base)
{
	// We check for an initially valid integer, since there is no
	// way in the interface to signal invalid string.  Thus we can
    // catch unexpected input at the cost of requiring valid input.
	const char*	p = s;

	assert( s != NULL );
	assert( base == 0 || (base >= 2 && base <= 36) );
		// can have leading white-space
	while( *p && isspace(*p) ) ++p;
	if ( *p == '-' )
	{
		++p;
	}
	assert( *p != '\0' );
	if ( base == 0 || base == 10 )
	{
		assert( isdigit(*p) );
	}
	else
	{
		const char*	dp;
		static const char	digs[] = "0123456789abcdefghijklmnopqrstuvwxyz";
		assert( sys_strlen(digs) == 36 );
		assert( isalnum(*p) );
		assert( base > 1 );
		assert( base <= sys_strlen(digs) );
		dp = sys_strchr(digs, tolower(*p));
		assert( dp != NULL );
		assert( dp >= digs );
		assert( (unsigned int)(dp - digs) < base );
	}
}

//=============================================================================

int
sys_atoi(const char* s)
{
	assert( s != NULL );
	_checkDigits(s, 0);
	return _SYS_CAST_2INT atoi((char*)s);
}


long
sys_atol(const char* s)
{
	assert( s != NULL );
	_checkDigits(s, 0);
	return atol((char*)s);
}


long
sys_strtol(const char* s, char** endp, int base)
{
	assert( s != NULL );
	assert( base == 0 || (base >= 2 && base <= 36) );
	_checkDigits(s, base);
	return strtol((char*)s, endp, base);
}

unsigned long
sys_strtoul(const char* s, char** endp, int base)
{
	assert( s != NULL );
	assert( base == 0 || (base >= 2 && base <= 36) );
	_checkDigits(s, base);
	return strtoul((char*)s, endp, base);
}

#endif	//DO_ASSERTIONS

//==============================================================================

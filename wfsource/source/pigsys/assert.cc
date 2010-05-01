//===========================================================================*/
// assert.cc
//===========================================================================*/

#include "pigsys.hp"
#include <cstdio>
#if defined(__WIN__)
#include <conio.h>
#endif
#include <cstring>
#include <cstdlib>
#undef abort

#pragma message( "TODO: HAL should probably provide an interface to the debugger [very simple to start with--like is it there?]" )
#if defined( __WIN__ )
#include <windows.h>
extern "C" BOOL WINAPI IsDebuggerPresent(void);
#include <cpplib/range.hp>
#if defined( ASSERT_DEBUG_MENU )
#include <menu/menu.hp>
#endif
#endif

//===========================================================================*/
// define a method for triggering the debugger
// when a debugger is present.

#if defined( __WIN__ )
#	if defined( __WATCOMC__ )
	extern void DebugInterrupt( void );
#		pragma aux DebugInterrupt = "int 3";
#	elif defined( _MSC_VER )
#		define DebugInterrupt()
#	else
#		include <dos.h>
#		define DebugInterrupt() 			\
		{									\
			 union REGS regs;				\
			 int386( 0x03, &regs, &regs );	\
		}
#	endif
#endif

#if defined(__PSX__)
void
DebugInterrupt()
{
	for(;;)
		;					// there is no OS to return to, why bother
}
#endif

#if defined( ASSERT_DEBUG_MENU )
const int MAX_COMMAND_LINES = 3;
static char* commandLines[MAX_COMMAND_LINES] =
{
	"Ignore the error and continue [dangerous!]",
	"Abort the program",
	"Launch debugger"
};
static char* descStrings[MAX_COMMAND_LINES] =
{
	"&Ignore",
	"&Abort",
	"&Debug"
};
#endif

enum { ASSERT_IGNORE=0, ASSERT_ABORT, ASSERT_DEBUG };

void
_sys_assert( int, const char* expr, const char* file, int line )
{
	char szLineNumber[ 15 ];
	snprintf( szLineNumber, 15, "%d", line );
#if defined( __WIN__)	
	if(file)
		_strlwr((char*)file);


	printf( "ÚÄ ASSERTION FAILED ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n" );
	if ( expr )
		printf( "³%-77s³\n", expr );
	printf( "³in file \"%s\" on line %s%*s³\n",	file, szLineNumber, 58-strlen(file)-strlen(szLineNumber), "" );
    printf( "ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n" );
#else
	printf( "+- ASSERTION FAILED ----------------------------------------------------------+\n" );
	if ( expr )
		printf( "|%-77s|\n", expr );
	printf( "|in file \"%s\" on line %s%*s|\n",	file, szLineNumber, 58-strlen(file)-strlen(szLineNumber), "" );
    printf( "+-----------------------------------------------------------------------------+\n" );

#endif		


#if defined( __WIN__ )
	{
	int nOptions = 2;
	if ( IsDebuggerPresent() )
		++nOptions;

	Range_Wrap range( 0, nOptions );
	range = int( ASSERT_IGNORE );
	char szFooter[ 256 ];
	snprintf( szFooter, 256, "File \"%s\" Line #%d", file, line );
#  if defined( ASSERT_DEBUG_MENU )
//        int idxOption = SimpleMenu( expr, szFooter, descStrings, commandLines, range, IsDebuggerPresent() ? ASSERT_DEBUG : ASSERT_ABORT );
        int idxOption = SimpleMenu( expr, szFooter, descStrings, commandLines, range, ASSERT_ABORT );
	if ( idxOption == ASSERT_ABORT )
		abort();
	else if ( idxOption == ASSERT_IGNORE )
		return;
	else if ( idxOption == ASSERT_DEBUG )
		DebugInterrupt();
#  else
        DebugInterrupt();
		abort();
#  endif
	}
#else
	exit( -1 );
#endif
}

//===========================================================================*/

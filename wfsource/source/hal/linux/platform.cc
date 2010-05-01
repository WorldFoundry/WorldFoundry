//==============================================================================
// platform.cc: windows95 specific startup code
// Copyright ( c ) 1997,98,99 World Foundry Group.  
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
// Original Author: Kevin T. Seghetti
//	History:
//			Created 03-07-95 11:45am Kevin T. Seghetti
//=============================================================================
// dependencies

#define _PLATFORM_C

//=============================================================================

// kts where is PATH_MAX comming from (it is not in limits.h)
//#include <limits.h>

#include <hal/hal.h>			// includes everything
#include <hal/_platfor.h>
#include <hal/_tasker.h>
#include <hal/_input.h>
#include <hal/sjoystic.h>
#include <hal/_message.h>
#include <hal/salloc.hp>
#include <signal.h>
extern bool bPrintVersion;

//=============================================================================

extern bool bShowWindow;

bool bFullScreen = true;

//=============================================================================

/*static*/ void PIGSInitStartupTask();

SAlloc* stacks;

//=============================================================================
// this is the actual main, which may be pasted into user code if you wish to
// modify it, or you can just default to this one

//=============================================================================

int		nMainCmdShow;
char	szAppName[ PATH_MAX ];
int		_halWindowWidth;
int		_halWindowHeight;
int		_halWindowXPos;
int		_halWindowYPos;

#define HALMEM	   	"-halmem="
#define SCRATCHMEM	"-scratchmem="
#define PSXMEM		"-psxmem"


void
ParseWindowSwitches( int __argc, char* __argv[] )
{ // Determine screen/window dimensions
#if 0
	for ( int i=1; i<__argc; ++i )
	{
		const char szWidth[] = "-width=";
		const char szHeight[] = "-height=";
		const char szXPos[] = "-xpos=";
		const char szYPos[] = "-ypos=";
		const char szWindow[] = "-window";
		const char szFullScreen[] = "-fullscreen";

		if ( 0 )
			;
		else if ( strcmp( __argv[ i ], szWindow ) == 0 )
			bFullScreen = false;
		else if ( strcmp( __argv[ i ], szFullScreen ) == 0 )
			bFullScreen = true;
		else if ( strnicmp( __argv[i], szXPos, strlen( szXPos ) ) == 0 )
			_halWindowXPos = atoi( __argv[i] + strlen( szXPos ) );
		else if ( strnicmp( __argv[i], szYPos, strlen( szYPos ) ) == 0 )
			_halWindowYPos = atoi( __argv[i] + strlen( szYPos ) );
		else if ( strnicmp( __argv[i], szWidth, strlen( szWidth ) ) == 0 )
			_halWindowWidth = atoi( __argv[i] + strlen( szWidth ) );
		else if ( strnicmp( __argv[i], szHeight, strlen( szWidth ) ) == 0 )
			_halWindowHeight = atoi( __argv[i] + strlen( szHeight ) );
#if defined( DESIGNER_CHEATS )
		else if ( strncmp( __argv[i], HALMEM, strlen( HALMEM ) ) == 0 )
			cbHalLmalloc = atoi( __argv[i] + strlen( HALMEM ) );
		else if ( strncmp( __argv[i], SCRATCHMEM, strlen( SCRATCHMEM ) ) == 0 )
			cbHalScratchLmalloc = atoi( __argv[i] + strlen( SCRATCHMEM ) );
		else if ( strncmp( __argv[i], PSXMEM, strlen( PSXMEM ) ) == 0 )
		{
			cbHalLmalloc = 1585296;
		}
#endif
	}

	if ( _halWindowHeight )
	{
		if ( _halWindowWidth == 0 )
		{	// Specified the height. Choose width accordingly
			switch ( _halWindowHeight )
			{
				case 200:
				case 240:
					_halWindowWidth = 320;
					break;
				case 384:
					_halWindowWidth = 512;
					break;
				case 400:
				case 480:
					_halWindowWidth = 640;
					break;
				case 600:
					_halWindowWidth = 800;
					break;
				case 768:
					_halWindowWidth = 1024;
					break;
				case 864:
					_halWindowWidth = 1152;
					break;
				case 1024:
					_halWindowWidth = 1200;
					break;
				case 1200:
					_halWindowWidth = 1600;
					break;
				default:
					printf( "Unknown height of %d\n", _halWindowHeight );
			}
		}

	}

#endif
	if ( _halWindowWidth == 0 )
	{
		_halWindowWidth = 512;
		_halWindowHeight = 384;
	}

}


int
main( int argc, char* argv[] )
{
	sys_init( &argc, &argv );

	assert( argv[0] );
	//_splitpath( argv[0], NULL, NULL, szAppName, NULL );
    strcpy(argv[0],szAppName);              // kts 4/10/99 8:52 
#if DEBUG
	strlwr( szAppName );
#endif

	ParseWindowSwitches( argc, argv );
#if		defined(DESIGNER_CHEATS)
	if ( bPrintVersion )
		printf( "cbHalLmalloc = %ld, cbHalScratchLmalloc = %ld\n", cbHalLmalloc, cbHalScratchLmalloc );
#endif

	HALStart(argc, argv,HAL_MAX_TASKS,HAL_MAX_MESSAGES,HAL_MAX_PORTS);

//	PIGSMain( argc, argv );
	return 0;
}

//=============================================================================

void* halMemory;

#if 0
void FPEHandler __PMT((int sig, siginfo_t *siginfo, void *undefined))
#else
void FPEHandler (int sig)
#endif
{
    // kts figure out how to return max int
    printf("FPEHandler, I got a signal %d\n",sig);
    printf("This should not have happened, the divide code in math/linux/scalar.hpi is supposed to prevent it!\n");
#if 0
    printf("siginfo = %p\n",siginfo);
    printf("undefined = %p\n",undefined);

    if(siginfo)
    {
        printf("si_signo = %d\n", siginfo->si_signo);
        printf("si_errno = %d\n", siginfo->si_errno);
        printf("si_code = %d\n", siginfo->si_code);
        printf("si_addr = %p\n", siginfo->si_addr);
    }
    printf("Someone figure out how to set ax to maxint and proceed\n");
#endif
// kts: don't run normal shutdown from an interrupt, just bail
#undef exit
    exit(5);
}

//==============================================================================

void
_PlatformSpecificInit(int /*argc*/, char** /*argv*/, int /*maxTasks*/,int /*maxMessages*/, int /*maxPorts*/)
{
    // install div 0/int overflow handler
#if 0
    struct sigaction act;
    act.sa_sigaction = FPEHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGFPE,&act, 0); 
#else
    struct sigaction act;
    act.sa_handler = FPEHandler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGFPE,&act, 0); 
#endif

//	halMemory = malloc(HALLMALLOC_SIZE);
	halMemory = malloc( cbHalLmalloc );
	ValidatePtr(halMemory);
	_HALLmalloc = new LMalloc(halMemory, cbHalLmalloc MEMORY_NAMED( COMMA "HalLMalloc" )	);
	assert(ValidPtr(_HALLmalloc));

	_HALDmalloc = new (*_HALLmalloc)DMalloc( *_HALLmalloc, HAL_DMALLOC_SIZE MEMORY_NAMED( COMMA "HALDmalloc"));
	ValidatePtr(_HALDmalloc);
}

//=============================================================================

void
_PlatformSpecificUnInit(void)
{
	assert(stacks);
	delete stacks;
	stacks = NULL;
	MEMORY_DELETE((*_HALLmalloc),_HALDmalloc,DMalloc);
	delete _HALLmalloc;
	free(halMemory);
}

//=============================================================================

void
FatalError( const char* string )
{
	printf("Fatal Error: %s",string);
	exit(1);
}

//=============================================================================

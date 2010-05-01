//==============================================================================
// main.cc:
// Copyright (c) 1994,1995,1996,1997,1999,2000,2001,2002,2003 World Foundry Group  
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
// Description: The main entry point of game engine
// Original Author: Ann Mei Chang
//==============================================================================

#define _MAIN_CC

#include <pigsys/pigsys.hp>
#include "game.hp"
#include "version.hp"

#if defined( __WIN__ ) || defined(__LINUX__)
	char szOadDir[ _MAX_PATH ];
#endif

#if defined ( __PSX__ )
#include <libgte.h>
#include <libgpu.h>
#if DBSTREAM < 1
bool enableScreenStream = false;        // kts remove in final game, used
#endif
#endif

#include "level.hp"

#if defined( __WIN__ )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
int streamFrom = 0;
extern "C" BOOL WINAPI IsDebuggerPresent(void);
#endif

#include <pigsys/pigsys.hp>
#include <cpplib/libstrm.hp>

#include "gamestrm.hp"

//==============================================================================

#if defined(__PSX__)
BeforeMain wfBeforeMain;
#endif
bool gSoundEnabled = false;
bool gCDEnabled = false;

WFGame* theGame = NULL;

#if defined(DESIGNER_CHEATS)
bool bRecordTGA = false;
#endif

#if defined( __PSX__ )
bool bRenderZs = true;
#else
bool bPerspectiveCorrection = false;
bool bRenderZs = false;
#endif
bool bRenderZb = !bRenderZs;

bool bLinearMalloc = false;

#if defined(DO_PROFILE)
bool bProfileMemLoad = false;
bool bProfileMainLoop = true;
#endif
bool printFrameRate = false;

// Fake value for simulating a constant framerate (for debugging only!)
Scalar FakeFrameRate = SCALAR_CONSTANT(0);				// can't use this in global space ->Scalar::zero;

bool bUsePrepreparedModels = true;	// -Tpsx should change this
bool bShowWindow = true;

#if defined(JOYSTICK_RECORDER)
#pragma message("INCLUDING JOYSTICK RECORDER CODE")
bool bJoyPlayback = false;		// Set true to enable joystick playback
std::ifstream* joystickInputFile;	// istream for joystick data
#endif


const char szRate[] = "rate";

#if ( !defined( __PSX__ ) && SW_DBSTREAM > 0)

void
usage( int argc, char* argv[] )
{
   (void) argc;
   (void) argv;
	std::cout << "Usage : " << __GAME__ << " {switches} <level #>" << std::endl;
	std::cout << "Switches:" << std::endl;
	std::cout << "\t-p<stream initial><stream output>, where:" << std::endl;
	std::cout << "\t\t<stream initial> can be any of:" << std::endl;
	std::cout << "\t\t\tw=warnings (defaults to standard err)" << std::endl;
	std::cout << "\t\t\te=errors (defaults to standard err)" << std::endl;
	std::cout << "\t\t\tf=fatal (defaults to standard err)" << std::endl;
	std::cout << "\t\t\ts=statistics (defaults to null)" << std::endl;
	std::cout << "\t\t\tp=progress (defaults to null)" << std::endl;
	std::cout << "\t\t\td=debugging  (defaults to null)" << std::endl;
	std::cout << "\t\t<stream output> can be any of:" << std::endl;
	std::cout << "\t\t\tn=null, no output is produced" << std::endl;
	std::cout << "\t\t\ts=standard out" << std::endl;
	std::cout << "\t\t\te=standard err" << std::endl;
#if defined (__PSX__)
	std::cout << "\t\t\tc=screen, display on psx screen" << std::endl;
	std::cout << "\t\t\tu=screenflush, display on psx screen during startup" << std::endl;
#endif
	std::cout << "\t\t\tm#=monochrome dispay, # = which window" << std::endl;
	std::cout << "\t\t\tf<filename>=output to filename" << std::endl;
	std::cout << "\t-s<stream initial><stream output>, where:" << std::endl;
	std::cout << "\t\t<stream initial> can be any of:" << std::endl;

#define STREAMENTRY(stream,where,initial,helptext) std::cout << "\t\t\t" << initial << "=" << helptext << std::endl;
#define EXTERNSTREAMENTRY(stream,where,initial,helptext) std::cout << "\t\t\t" << initial << "=" << helptext << std::endl;
#include "gamestrm.inc"
#undef STREAMENTRY
#undef EXTERNSTREAMENTRY

	std::cout << "\t\t<stream output> (same as above)" << std::endl;

	std::cout << "\t-l<stream initial><stream output>, where:" << std::endl;
	std::cout << "\t\t<stream initial> can be any of:" << std::endl;
#define STREAMENTRY(stream,where,initial,helptext) std::cout << "\t\t\t" << initial << "=" << helptext << std::endl;
#include <libstrm.inc>
#undef STREAMENTRY
	std::cout << "\t\t<stream output> (same as above)" << std::endl;

    std::cout << "\t-paranoid\tPerform insanely slow error checks" << std::endl;
    std::cout << "\t-record_tga\tSave every frame as frame#.tga in cwd" << std::endl;
#if !defined( __PSX__ )
    std::cout << "\t-zs\t\tZ-Sorted" << std::endl;
    std::cout << "\t-zb\t\tZ-Buffered" << std::endl;
#endif
#if defined( __WIN__ )
	std::cout << "\t-window\t\tDisplay game in a window" << std::endl;
#endif
	std::cout << "\t-nologo\t\tDon't display company logos" << std::endl;
//			std::cout << "\t-framerate\tPrint framerate" << std::endl;
#if defined (__WIN)
	std::cout << "\t-perspective\tEnable texture perspective correction" << std::endl;
#endif
	std::cout << "\t-" << szRate << "N\t\tSimulate a frame rate of N Hz" << std::endl;
	std::cout << "\t-profmemload\tProfile memory usage during load" << std::endl;
	std::cout << "\t-profmainloop\tProfile cpu usage during main game loop" << std::endl;
	std::cout << "\t-lmalloc\tUse linear malloc() instead of C runtime malloc()" << std::endl;
	std::cout << "\t-sound\t\tEnable Sound" << std::endl;
	std::cout << "\t-cd\t\tenable CD" << std::endl;
#if defined(JOYSTICK_RECORDER)
	std::cout << "\t-joy<filename>\tPlay back joystick input from <filename>" << std::endl;
#endif
#if DO_DEBUGGING_INFO
	std::cout << "\t-breaktime<time>\tcause debugger break at time <time>" << std::endl;
#endif
	DBSTREAM1( std::cout << "CALLING PIGSExit()" << std::endl; )
	PIGSExit();
}

#endif

//==============================================================================
// command line parsing
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(int argc, char** argv)
{
	int index;
	for( index=1; index < argc && argv[index] && ((*argv[index] == '-') || (*argv[index] == '/')); index++)
	{
#if 0
#if defined( __WIN__ )
		const char szHeight[] = "height";
#endif
#endif

		DBSTREAM1( std::cout << argv[index] << std::endl; )

		if ( 0 )
			;
		else if ( strncmp( argv[index]+1, (char*)szRate, strlen( szRate ) ) == 0)
		{
		    int value = atoi( argv[index] + strlen( szRate ) + 1 );
          FakeFrameRate = Scalar::one / Scalar(value,0);
			FakeFrameRate = SCALAR_CONSTANT(0.05);
			DBSTREAM1( cerror << "Fake clock delta = " << FakeFrameRate << std::endl; )
		}
#if defined( __WIN__ )
		else if ( strcmp( argv[index]+1, "perspective" ) == 0 )
			bPerspectiveCorrection = true;
#endif
#if defined(DESIGNER_CHEATS)
        else if ( strcmp( argv[index]+1, "record_tga" ) == 0 )
            bRecordTGA = true;
#endif
#if !defined( __PSX__ )
		else if ( strcmp( argv[index]+1, "zb" ) == 0 )
		{
			bRenderZb = true;
			bRenderZs = false;
		}
		else if ( strcmp( argv[index]+1, "zs" ) == 0 )
		{
			bRenderZs = true;
			bRenderZb = false;
		}
#endif
#if defined( __WIN__ )
		else if ( strcmp( argv[ index ] + 1, "hd" ) == 0 )
			streamFrom = 1;
		else if ( strcmp( argv[ index ] + 1, "cd" ) == 0 )
			streamFrom = 2;
#endif
#if defined(JOYSTICK_RECORDER)
		else if ( memcmp( argv[index]+1, "joy", 3 ) == 0 )
		{
			AssertMsg( strlen(argv[index]+1) > 3, "The -joy option requires a filename" );
			bJoyPlayback = true;
			joystickInputFile = new (HALLmalloc) std::ifstream(argv[index]+4);
			AssertMsg( joystickInputFile->good(), "Unable to open file <" << argv[index]+4 << "> for joystick input" );
		}
#endif
#if defined(DO_PROFILE)
		else if ( strcmp( argv[index]+1, "profmemload" ) == 0 )
			bProfileMemLoad = true;
		else if ( strcmp( argv[index]+1, "profmainloop" ) == 0 )
			bProfileMainLoop = true;
#endif
#if DO_DEBUGGING_INFO
		else if ( memcmp( argv[index]+1, "breaktime=", 10 ) == 0 )
		{
			AssertMsg( strlen(argv[index]+1) > 10, "The -breaktime= option requires a time" );
			extern Scalar WALL_CLOCK_BREAKPOINT_VALUE;
#if defined(SCALAR_TYPE_FIXED)
         long value = atoi( argv[index] + 1 + 10 );
        WALL_CLOCK_BREAKPOINT_VALUE = Scalar::FromFixed32(value);
#elif defined(SCALAR_TYPE_FLOAT) || defined(SCALAR_TYPE_DOUBLE)
         double value;
         sscanf(argv[index] + 1 + 10, "%lf",&value);
         WALL_CLOCK_BREAKPOINT_VALUE = Scalar::FromDouble(value);
#else
#error SCALAR TYPE not defined
#endif

		}
#endif
#if DEBUG > 0
        else if ( strcmp( argv[index]+1, "lmalloc" ) == 0 )
            bLinearMalloc = true;
		else if ( strcmp( argv[index]+1, "sound" ) == 0 )
		{
			gSoundEnabled = true;
            gCDEnabled = true;
			//std::cout << "Sound is OFF" << end1;
		}
        else if ( strcmp( argv[index]+1, "cd" ) == 0 )
		{
            gCDEnabled = true;
            //std::cout << "CD is OFF" << end1;
		}
#endif
#if SW_DBSTREAM > 0
		else if ( tolower( *( argv[index]+1 ) ) == 'p' )
			RedirectStandardStream(argv[index]+2);
		else if ( tolower( *( argv[index]+1 ) ) == 's' )
			RedirectGameStream(argv[index]+2);
		else if ( tolower( *( argv[index]+1 ) ) == 'l' )
			RedirectLibraryStream(argv[index]+2);
#endif

#pragma message ("kts: printframerate should be removed from release")
#if defined(DESIGNER_CHEATS)
		else if ( tolower( *( argv[index]+1 ) ) == 'f' )
		{
			  printFrameRate = true;
			  DBSTREAM1( std::cout << "PrintFrameRate on" << std::endl; )
	 	}
#endif
#if (!defined( __PSX__ ) && SW_DBSTREAM > 0)
		else if ( *( argv[index]+1 ) == 'h' )
	  	{
			usage( argc, argv );
         sys_exit(1);
  		}
#endif		// !defined( __PSX__ ) )
#if DO_ASSERTIONS
		else
			DBSTREAM1( cerror << "game Error: Unrecognized command line switch \"" <<
				*(argv[index]+1) << "\"" << std::endl );
#endif
	 }

	return(index);
	}



#if defined( __PSX__ )
extern int   _ramsize;
extern int _stacksize;
extern int __heapsize;
extern void* __heapbase;
#endif

#if defined( __WIN__ )
bool
GetLocalMachineStringRegistryEntry( const char* path, const char* valueName, char* contents, int sizeOfContents )
{
	HKEY resultKey = NULL;
    PHKEY phkResult = &resultKey; 	// address of handle of open key

//	                              KEY_ENUMERATE_SUB_KEYS |
//                              KEY_EXECUTE |

	long retVal = RegOpenKeyEx( HKEY_LOCAL_MACHINE,	path, 0,
		KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, phkResult );
	//std::cout << "Registry read, retVal = " << retVal << ", key = " << resultKey << std::endl;
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( retVal == ERROR_SUCCESS );
	assert( phkResult );
	assert( resultKey );

	DWORD type;
	DWORD cbData = sizeOfContents;

	retVal = RegQueryValueEx( resultKey, valueName, NULL, &type, (BYTE*)contents, &cbData );
	//std::cout << "retVal = " << retVal << std::endl;
	//std::cout << "type = " << type << std::endl;
	if ( retVal != ERROR_SUCCESS )
		return false;
	assert( type == REG_SZ );
	assert( retVal == ERROR_SUCCESS );
	return true;
}
#endif


void
PIGSMain( int argc, char* * argv )
{
#if 0	//msvc defined( TASKER )
	assert( TaskGetPriority(GetCurrentTask()) == 0);
	if( TaskGetPriority(GetCurrentTask()) != 0)
		printf("Incorrect task priority, game will probably fail\n");
#endif

//	ERR_SET_LOG(3);

//	std::cout << "Heap Base =" << __heapbase << ", heapsize = " << __heapsize << std::endl;
//#if DO_ASSERTIONS && defined( __PSX__ )
//	void* test = malloc(10000000);		// allocate 10 million bytes (this should fail)
//	AssertMsg(test == 0,"Wrong malloc in use, should have returned 0, returned " << test);
//#endif

	AssertMsg( true & 1, "True must have low bit set [required for bitfield memory optimization]" );

#if defined( __WIN__ )
	bool bSuccess = GetLocalMachineStringRegistryEntry( "SOFTWARE\\World Foundry\\GDK", "OAD_DIR", szOadDir, sizeof( szOadDir ) );
#pragma message( __FILE__ ": check to see what happens if szOadDir not gotten from registry" )
	//AssertMsg( bSuccess, "\"OAD_DIR\" not set in registry (SOFTWARE\\World Foundry\\GDK\\OAD_DIR)" );
#endif

	DBSTREAM1( std::cout << __GAME__ << " v" << (char*)szVersion; )

	DBSTREAM1( std::cout << ", Built:" << (char*)szDate << "," << (char*)szTime << " by " << szBuildUser << std::endl; )
	int commandIndex = ParseCommandLine(argc,argv);

#if defined( __WIN__ )
	// default to 20fps if in debugger
	if ( IsDebuggerPresent()
#if defined(JOYSTICK_RECORDER)
	&& !bJoyPlayback
#endif
	)
			FakeFrameRate = Scalar::one / Scalar(20,0);
#endif

#if defined(JOYSTICK_RECORDER)
	// check for conflicting time-related options
	if (FakeFrameRate != Scalar::zero)
		AssertMsg( !bJoyPlayback, "-joy and -rate options can't be used at the same time." );
	if(bJoyPlayback)
		printf("Playing back joystick file\n");
#endif

#pragma message ("kts: should not read level from command line (leave until profiling is done!)")
	int nStartingLevel = -1;
	if (argc-commandIndex > 0)
		nStartingLevel = atoi( argv[ commandIndex ] );
	DBSTREAM1( std::cout << "argc = " << argc << ", commandIndex = " << commandIndex << std::endl; )
	DBSTREAM1( cver << "Level=" << nStartingLevel << std::endl; )

#if 0
#if defined( __WIN__ )
	{
		int error;
		for ( int i=0; i<10; ++i )
		{
			JOYINFO ji;
			if ( ( error = joyGetPos( 0, &ji ) ) != JOYERR_NOERROR )
				break;
		}
		AssertMsg( error == JOYERR_NOERROR, "Joystick driver not installed or joystick not plugged in" );
	}
#endif
#endif

#pragma message( "System streams should be in HAL [framework]" )
#if SW_DBSTREAM > 0
	cerror << "cerror:" << std::endl;
	cwarn << "cwarn:" << std::endl;
	cstats << "cstats:" << std::endl;
	cprogress << "cprogress:" << std::endl;
	cdebug << "cdebug:" << std::endl;

	cflow << "cflow:" << std::endl;
	cmovement << "cmovement:" << std::endl;
	cactor << "cactor:" << std::endl;
	clevel << "clevel:" << std::endl;
	ctool << "ctool:" << std::endl;
	ccamera << "ccamera:" << std::endl;
	cmem << "cmem:" << std::endl;
#endif
#if SW_DBSTREAM > 0
#	ifdef __PSX__
	DBSTREAM1( cver = std::cout; )
#	endif
	DBSTREAM1( cver << __GAME__ " Ver " << (char*)szVersion << ", Built:" << (char*)szDate << "," << (char*)szTime << " by " << szBuildUser << std::endl; )
	DBSTREAM1( cver << "DBSTREAMS "; )
#	ifdef DO_PROFILE
	cver << "PROFILE ";
#	endif
#endif

	DBSTREAM1( cprogress << "main::constructing the game" << std::endl; )
   WFGame* game = new (HALLmalloc) WFGame( nStartingLevel );
	assert( ValidPtr( game ) );
   theGame = game;
	DBSTREAM1( cprogress << "main::running game script" << std::endl; )

	game->RunGameScript( );

	DBSTREAM1( cprogress << "Game over: shutting down the game" << std::endl; )

	MEMORY_DELETE(HALLmalloc,game,WFGame);

#if defined(JOYSTICK_RECORDER)
	if (bJoyPlayback)
		MEMORY_DELETE( HALLmalloc, joystickInputFile, ifstream );
#endif

	DBSTREAM1( cprogress << "Calling PIGSExit()" << std::endl; )
	PIGSExit();
}

//==============================================================================

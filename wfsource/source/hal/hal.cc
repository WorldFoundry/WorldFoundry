//============================================================================
// hal.cc: startup code/entry point for HAL
//============================================================================
// Documentation:
//
//	Abstract:
//		Boots the PIGS operating system, contains HalMain
//		actual main is in each platform specific platform.cc, since some
//		OS's (like Windows) don't call main, they enter elsewhere
//
//	History:
//		Created 01-11-95 01:17pm Kevin T. Seghetti
//
//============================================================================
// dependencies

#define _PIGS_C

#include <hal/hal.h>
#include <hal/_platfor.h>
#include <hal/_tasker.h>
#include <hal/_input.h>
#include <hal/sjoystic.h>
#include <hal/_message.h>
#include <hal/diskfile.hp>

//============================================================================

static void PIGSInitStartupTask();

LMalloc* _HALLmalloc;
LMalloc* _HALScratchLmalloc;
DMalloc* _HALDmalloc;
int32 cbHalLmalloc = 3500000;
int32 cbHalScratchLmalloc = 180000;

//============================================================================
// main HAL entry point, starts up PIGS system, including tasker & timers
// pass in argc & argv so that PIGSMain will have them
// this portion of the startup code has the entire machine, the tasker is not
//	running yet
//----------------------------------------------------------------------------

void
HALStart(int argc, char** argv, int maxTasks,int maxMessages, int maxPorts)
{
//	printf("HAL Argv Dump: argc = %d\n",argc);
//	for(int argvIndex=0;argvIndex < argc; argvIndex++)
//		printf("  %d: <%s>\n",argvIndex,argv[argvIndex]);

//#if !FINAL_RELEASE
//	for ( int idxArg=0; idxArg<argc; ++idxArg )
//	{
//		//printf( "argv[%d] = %s\n", idxArg, argv[idxArg] );
//		if ( strcmp( argv[ idxArg ], "-nocd" ) == 0 )
//			bInitCd = false;
//	}
//#endif

	_PlatformSpecificInit( argc, argv, maxTasks, maxMessages,  maxPorts );
	assert(ValidPtr(_HALLmalloc));
	{
		LMalloc __scratchLMalloc(*_HALLmalloc,cbHalScratchLmalloc MEMORY_NAMED( COMMA "HalScratchLMalloc" ) );
		_HALScratchLmalloc = &__scratchLMalloc;
		assert(ValidPtr(_HALScratchLmalloc));
		HalInitFileSubsystem();
		_InitJoystickInterface();					// setup joystick code
#if DO_VALIDATION
		ItemInit();
#endif
#if defined( DO_MULTITASKING )
		_MessageSystemConstruct(maxMessages,maxPorts);				// this is where the malloc occurs
		_PublicMessagePortListConstruct();
		_TaskerStart(PIGSInitStartupTask,maxTasks,maxMessages,maxPorts);				// note: doesn't return until someone calls _TaskerEnd
#else
		PIGSMain( __argc, __argv );
#endif

#if defined( DO_MULTITASKING )
	// cleanup
		_PublicMessagePortListDestruct();
		_MessageSystemDestruct();
#endif
#if DO_VALIDATION
		ItemTerm();
#endif
		_TermJoystickInterface();
	}
	_PlatformSpecificUnInit();
}

//----------------------------------------------------------------------------
// this portion of the startup code runs as a task

void
PIGSInitStartupTask()
{
	// any pigs modules which want to always have a task running should start it here
#if defined( TASKER )
//	_TimerStart(HAL_MAX_TIMER_SUBSCRIBERS);			// create all timer tasks
#endif
	PIGSMain( __argc,__argv );				// call game code, when returns, game is over
	PIGSExit();
}

//============================================================================
// shut down multi-tasker and call exit

void
PIGSExit()
{
#if defined( DO_MULTITASKING )
//	_TimerStop();
	_TaskerStop();			// shuts down tasker, doesn't return
#endif
#if DO_TEST_CODE
	printf("Tasker shutting down\n");
#endif
#if defined(__PSX__)
	assert(0);
#endif
}

//============================================================================

bool
PIGSUserAborted()				// returns true if user has aborted game
{
	return(_JoystickUserAbort());
}

//============================================================================

#if DO_DEBUGGING_INFO
void
breakpoint()
	{
	}
extern "C" { extern int bDebugger; }							// kts used to enable int3 in assert
#endif

//==============================================================================


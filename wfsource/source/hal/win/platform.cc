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

#define USE_WINMAIN 0

//=============================================================================

#pragma message( __FILE__ ": want to include hal.h first" )
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <time.h>
#include <malloc.h>

#include <hal/hal.h>			// includes everything
#include <hal/_platfor.h>
#include <hal/_tasker.h>
#include <hal/_input.h>
#include <hal/sjoystic.h>
#include <hal/_message.h>
#include <hal/salloc.hp>
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

#if USE_WINMAIN

HDC hdc;
HWND hwnd;
bool bActive;

long FAR PASCAL
WndProc( HWND _hwnd, UINT message, UINT wParam, LONG lParam )
{
	PAINTSTRUCT ps;
	RECT rect;

	switch ( message )
	{
		case WM_ACTIVATEAPP:
			bActive = bool( wParam );
			return 0;

		case WM_ERASEBKGND:
		{
			static int nTimesErased = 0;

			++nTimesErased;
			//assert( nTimesErased == 1 );

			return 0;
		}

		case WM_PAINT:
			hwnd = _hwnd;
			hdc = BeginPaint( hwnd, &ps );
			GetClientRect( hwnd, &rect );
			HALStart(__argc, __argv,HAL_MAX_TASKS,HAL_MAX_MESSAGES,HAL_MAX_PORTS);
			EndPaint( hwnd, &ps );
			// fall through to quit

		case WM_DESTROY:
			//GameShutdown();
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( _hwnd, message, wParam, lParam );
}

#endif

//=============================================================================

HANDLE	hMainInstance;
HANDLE	hMainPrevInstance;
int		nMainCmdShow;
char	szAppName[ _MAX_FNAME ];
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

	if ( _halWindowWidth == 0 )
	{
		_halWindowWidth = 640;
		_halWindowHeight = 480;
	}

}


int
main( int argc, char* argv[] )
{
	sys_init( &argc, &argv );

	assert( argv[0] );
	_splitpath( argv[0], NULL, NULL, szAppName, NULL );
	strlwr( szAppName );

	ParseWindowSwitches( argc, argv );
#if		defined(DESIGNER_CHEATS)
	if ( bPrintVersion )
		printf( "cbHalLmalloc = %ld, cbHalScratchLmalloc = %ld\n", cbHalLmalloc, cbHalScratchLmalloc );
#endif

	HALStart(argc, argv,HAL_MAX_TASKS,HAL_MAX_MESSAGES,HAL_MAX_PORTS);

//	PIGSMain( argc, argv );
	return 0;
}

#if USE_WINMAIN

int PASCAL
WinMain( HANDLE hInstance, HANDLE hPrevInstance, LPSTR /*lpszCmdParam*/, int nCmdShow )
{
	assert(0);
	// for export
	hMainInstance = hInstance;
	hMainPrevInstance = hPrevInstance;
	nMainCmdShow = nCmdShow;

	HANDLE hProc=GetCurrentProcess();
	SetPriorityClass(hProc,REALTIME_PRIORITY_CLASS);

	sys_init(&__argc, &__argv);

	const int32 totalStackSize = 1024*1024;				// kts how should this be controlled?
	// kts set up stack memory system
	void* stackMemory = alloca(totalStackSize);
	assert(ValidPtr(stackMemory));
	stacks = new SAlloc(stackMemory,totalStackSize);
	assert(ValidPtr(stacks));

	assert( __argv[0] );
	_splitpath( __argv[0], NULL, NULL, szAppName, NULL );
	strlwr( szAppName );

	ParseWindowSwitches( __argc, __argv );

	srand( time( NULL ) );

#if defined( _MSC_VER )
	PIGSMain( __argc, __argv );
	return 0;
#endif

	if( bShowWindow )
	{
		HWND hwnd;
		MSG msg;
		WNDCLASS wndclass;

		if( !hPrevInstance )
		{
			wndclass.style = CS_HREDRAW | CS_VREDRAW;
			wndclass.lpfnWndProc = WndProc;
			wndclass.cbClsExtra = 0;
			wndclass.cbWndExtra = 0;
			wndclass.hInstance = hInstance;
			wndclass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
			wndclass.hCursor = LoadCursor( NULL, IDC_ARROW );
			wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
			wndclass.lpszMenuName = NULL;
			wndclass.lpszClassName = szAppName;
			RegisterClass( &wndclass );
		}

		ShowCursor( FALSE );

		assert( _halWindowWidth > 0 );
		assert( _halWindowHeight > 0 );
		hwnd =
			CreateWindowEx
			(
				0,
				szAppName,
				szAppName,
				WS_OVERLAPPEDWINDOW,
//				0, 0, 320+8, 240+27,
				0, 0, _halWindowWidth+8, _halWindowHeight+27,
				NULL,
				NULL,
				hInstance,
				NULL
			);

		ShowWindow( hwnd, nCmdShow );
		UpdateWindow( hwnd );
		SetFocus( hwnd );

		for( ;; )
		{
			if ( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
			{
				if ( !GetMessage( &msg, NULL, 0, 0 ) )
					return msg.wParam;
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else if ( bActive )
			{
				//DrawScreen();
				//GameLoop();
			}
			else
				WaitMessage();
		}
		//AssertMsg( 0, "End of HAL" );

		//NOTREACHED
		return msg.wParam;
	}
	else
	{
		HALStart(__argc, __argv,HAL_MAX_TASKS,HAL_MAX_MESSAGES,HAL_MAX_PORTS);
		return 0;
	}
}

#endif

//=============================================================================

void* halMemory;

void
_PlatformSpecificInit(int /*argc*/, char** /*argv*/, int /*maxTasks*/,int /*maxMessages*/, int /*maxPorts*/)
{
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

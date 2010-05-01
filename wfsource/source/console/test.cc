//=============================================================================
// test.cc
//=============================================================================

#include <console/console.hp>
#include <hal/hal.h>
#include <console/hdump.hp>

bool bShowWindow = false;

#define MAX_COMMAND_LINES 7

char* commandLines[MAX_COMMAND_LINES] =
{
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
};

// kts help to indicate which buttons to press
char* descStrings[MAX_COMMAND_LINES] =
{
	"(none)     = menu        ",
	"Tri	    = 3D text     ",
	"Circle     = cyberthug   ",
	"Tri+Circle = snowgoons   ",
	"X          = streets     ",
	"X+Tri      = whitestar   ",
	"X+Circle   = minecart    "
};



int
Menu()
{
	Console console;

	console.print( "\nWorld Foundry Level Loader\n\nHold down level select buttons\n then press start\n\n" );

	for ( int index=0; index<MAX_COMMAND_LINES; ++index )
	{
		console.print( descStrings[index] );
		console.print( commandLines[index] );
		console.print( "\n\n" );
	}

	char szRamSize[ 40 ];
	int _ramsize = 4*1024*1024L;
	sprintf( szRamSize, "_ramsize = %ld [%dMB]", _ramsize, _ramsize / (1024L * 1024L) );
	console.print( "\n" );
	console.print( szRamSize );
	console.print( "\n\n" );

	console.flush();

#if 0
	PadInit(0);
	while(!(PadRead(0) & PADstart))
		;

	return (PadRead(0) >> 4) & 0xf;
#else
	// wait
	return 1;
#endif
}



void
SectorViewer()
{

}

//=============================================================================

void
main(int argc, char* argv[])
{
#if 1
	Menu();

	const char szMsg[] = "Here is some text for the hdump command!";

	Console console;
	HDump( "Title", (void*)szMsg, strlen( szMsg ), console );
//	console.flush();
#endif

	SectorViewer();

//	PIGSExit();
}

//=============================================================================

//=============================================================================
// platform.c: psx specific startup code
//=============================================================================
// Documentation:
//	Abstract:
//		Does whatever is required to interface to target platform

//	History:
//			Created 03-07-95 11:45am Kevin T. Seghetti

//	Class Hierarchy:
//			none

//	Dependancies:
//		All of the HAL
//	Restrictions:

//	Example:

//=============================================================================
// dependencies

#define _PLATFORM_C

#define HAL_SELECT_LEVEL 0

#include <hal/hal.h>			// includes everything

extern "C" {
#	include <r3000.h>
#	include <asm.h>
#	include <kernel.h>
#	include <libetc.h>
#	include <stdio.h>
#	include <libsn.h>
#	include <libgte.h>
#	include <libgpu.h>

#	include <missing.h>
#	include <libsn.h>

#	include <except.h>

#if defined( SOUND )
#	include <libspu.h>
#endif
	};

//#include <streams/str_type.hp>
#include <streams/dbstrm.hp>

//============================================================================

#if defined(DO_PROFILE)
int   _ramsize=0x00400000; // 4 Megabytes of main memory so there is enough to allocate the profile space
#else
#if DO_IOSTREAMS
int   _ramsize=0x00800000; // 8 Megabytes of main memory
#else
//int   _ramsize=0x00800000; // 8 Megabytes of main memory
int   _ramsize=0x00200000; // 2 Megabytes of main memory
#endif
#endif
int _stacksize=0x00002000; // and reserve 8K of that for stack
bool installExceptionHandler = 1;

LMalloc* scratchPadMemory;

//=============================================================================
// code I wish would run before main

BeforeMain beforeMain;			        // global instance


BeforeMain::BeforeMain()
{
	printf("beforemain ran\n");
	InitGeom();
}

//=============================================================================

INLINE
char* EatWhiteSpace(char* string)
{
	assert(string);
	while(*string && (*string == ' ' || *string == 0xa || *string == 0xd))
	 {
		if(*string == 0xa || *string == 0xd)
			*string = ' ';
		string++;
	 }
	return(string);
}

//=============================================================================

void
InitSimpleDisplay()
{
	ResetGraph(0);
	SetGraphDebug(0);
	FntLoad(640, 0);
   	SetDumpFnt(FntOpen(0,12, 320, 240, 0, 1024));
	//SetDumpFnt(FntOpen(0,12, 320, 240, 0, 1200));
	static DRAWENV g_ExDrawEnv;
	SetDefDrawEnv(&g_ExDrawEnv, 0, 0, 320, 240);
	static DISPENV g_ExDispEnv;
	SetDefDispEnv(&g_ExDispEnv, 0, 0, 320, 240);
	PutDrawEnv(&g_ExDrawEnv);
	PutDispEnv(&g_ExDispEnv);
}

//=============================================================================

void
UpdateSimpleDisplay()
{
	static RECT bg = {0, 0, 320, 240};

	ClearImage(&bg, 10, 10,50);
	FntFlush(-1);
	SetDispMask(1);
	DrawSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
	VSync(0);
}

//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================

char
charTable[256] =
{
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
	'0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
	'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
	'P','Q','R','S','T','U','V','W','X','Y','Z','[','\\',']','^','_',
	'`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o',
	'p','q','r','s','t','u','v','w','x','y','z','{','|','}','~','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.',
	'.','.','.','.','.','.','.','.','.','.','.','.','.','.','.','.'
};

//============================================================================

ulong
HDumpLine(char* buffer, ulong bufferSize)
{
	ulong i = bufferSize;
	char* dataBuffer = buffer;
	char ch;
	uint count = 0;
	while(i--)					// first print hex #'s
	 {
		FntPrint("%02x",*dataBuffer++);
		if(!((count+1) % 4))
			FntPrint(" ");
		count++;
	 }

	i = bufferSize;
	dataBuffer = buffer;
	while(i--)					// now print ascii
	 {
		ch = *dataBuffer++;
		FntPrint("%c",charTable[ch]);
	 }
	FntPrint("\n");
 	return(bufferSize);
}

//============================================================================

#define BYTES_PER_LINE 12
#define MAX_LINES 24
#define MAX_BUFFER_SIZE (BYTES_PER_LINE*MAX_LINES)

void
HDump_NoWait(void* buffer, ulong bufferSize)
{
	if(bufferSize > MAX_BUFFER_SIZE)
		bufferSize = MAX_BUFFER_SIZE;

	ulong len;
	ulong offset = 0;
	char* dataBuffer = (char*)buffer;
	assert(buffer);

//	printf("buffersize %lu\n", bufferSize);
	while(bufferSize)
	 {
		//FntPrint("%02x: ", offset);
		len = HDumpLine(dataBuffer,bufferSize >= BYTES_PER_LINE?BYTES_PER_LINE:bufferSize);
		dataBuffer += len;
		offset += len;
		bufferSize -= len;
	 }
}


void
HDump(char* title, void* buffer, ulong bufferSize)
{
	InitSimpleDisplay();
	FntPrint( "\n%s: addr = %x\n", title, buffer );
	HDump_NoWait(buffer,bufferSize);
	FntPrint("Press a button to continue");
	UpdateSimpleDisplay();
	PadInit(0);
	while(!(PadRead(0) & PADstart))
		;
	while((PadRead(0) & PADstart))
		;
}

//==============================================================================
//=============================================================================

#if !DO_DEBUG_FILE_SYSTEM
// the cheap joystick parsing I wrote only selects 0-15, so this can go up to 16
#define MAX_COMMAND_LINES 7

// kts kludge to allow command lines in the streaming version
#if DO_ASSERTIONS
char* commandLines[MAX_COMMAND_LINES] =
{
	"-rate20 0",
	"-rate20 1",
	"-rate20 2",
	"-rate20 3",
	"-rate20 4",
};
#else
char* commandLines[MAX_COMMAND_LINES] =
{
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
};
#endif


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


#endif

//=============================================================================

int		_halWindowWidth = 320;
int		_halWindowHeight = 240;
int		_halWindowXPos = 0;
int		_halWindowYPos = 0;

//=============================================================================

#include <hal/diskfile.hp>

void
DoFileReadTest()
{
	HalInitFileSubsystem();
	int sector=0;
	int offset=0;
#define BYTES_TO_DISPLAY 288
#define OFFSET_INC BYTES_TO_DISPLAY
	char stringBuffer[81];
	PadInit(0);
	InitSimpleDisplay();

	FntPrint( "\n\n -- DoFileReadTest -- \n Constructing DiskFile");
	UpdateSimpleDisplay();

	char discBuffer[DiskFileCD::_SECTOR_SIZE];
	char lmallocBuffer[3000];
	for(int index=0;index<DiskFileCD::_SECTOR_SIZE;index++)
		discBuffer[index] = 0;
	discBuffer[0] = 0x42;			// so I can tell if was written to

	LMalloc foo(lmallocBuffer, 3000 MEMORY_NAMED( COMMA "DoFileReadTest Memory" ) );	// placement lmalloc, you provide the memory
	_DiskFile* df = ConstructDiskFile("cd.iff",foo);

	FntPrint( "\n\n -- DoFileReadTest -- \n Done Constructing DiskFile");
	UpdateSimpleDisplay();

	while(1)
	{
		int position = sector * DiskFileCD::_SECTOR_SIZE;
//		printf("position = %d\n",position);
		df->SeekRandom( position );
		df->ReadBytes( discBuffer, DiskFileCD::_SECTOR_SIZE);
		sprintf(stringBuffer,"CD Read, sector: %d",sector);

		FntPrint( "\n%s: offset = %d\n", stringBuffer, offset );
		HDump_NoWait(discBuffer+offset,BYTES_TO_DISPLAY);
		FntPrint("Press up/down to select sectors");
		UpdateSimpleDisplay();

		while(!PadRead(0))
			;
		// ok, figure out which button was pressed
		long buttons = PadRead(0);

		if((buttons & PADLup) && sector > 0)
			sector--;

		if(buttons & PADLdown)
			sector++;

		if((buttons & PADLleft) && offset > 0)
			offset-=OFFSET_INC;

		if((buttons & PADLright) && (offset+OFFSET_INC) < DiskFileCD::_SECTOR_SIZE)
			offset+=OFFSET_INC;

		while(!PadRead(0))
			;
	}
}

//=============================================================================

void
DoSlateScreen()			// slate screen, added 06-17-98 05:11pm kts
{

	InitSimpleDisplay();
//									  |	<-- right edge marker for text
	FntPrint("\n\
        World Foundry Demo\n\
\n\
  World Foundry Release V3.1.6\n
\n\
  For Information contact:\n\
\n\
  World Foundry Group\n\
  11085 Iron Mountain Road\n\
  Redding, CA 96001\n\
\n\
  William B. Norris IV\n\
  wbniv@worldfoundry.org\n\
\n\
  Kevin T. Seghetti\n\
  kts@worldfoundry.org\n\
      \n\
  www.worldfoundry.org\n\
\n\
\n\
    >> Press start to begin <<\n\
\n\
\n\
 Copyright (c) 1998,99 World Foundry Group\n\
 All Rights Reserved\n\
	");
	static RECT bg = {0, 0, 320, 240};
#if defined(DO_CD_STREAMING)
	ClearImage(&bg, 200, 0,0);
#else
	ClearImage(&bg, 10, 10,50);
#endif
	FntFlush(-1);
	SetDispMask(1);
	DrawSync(0);
	VSync(0);

	PadInit(0);

	// kts hdump added 07-14-98 01:27pm

	while((PadRead(0) & PADstart))
		;
	while(!(PadRead(0) & PADstart))
	{
		if(PadRead(0) == (PADL1|PADL2|PADR1|PADR2))
			DoFileReadTest();
	}
	while(PadRead(0))				// wait for button release
		;
}

//=============================================================================
int
main()
{
	ResetCallback();

	DoSlateScreen();

	// kts command line disection
	const int MAX_ARGC = 30;
	char* argvPtrs[MAX_ARGC];


#if HAL_SELECT_LEVEL
	const int MAX_CMD_LINE = 300;
	char inputBuffer[MAX_CMD_LINE];
#if !DO_DEBUG_FILE_SYSTEM

	InitSimpleDisplay();

	FntPrint("\nWorld Foundry Level Loader\n\nHold down level select buttons\n then press start\n\n");

	for(int index=0;index<MAX_COMMAND_LINES;index++)
	{
		FntPrint(descStrings[index]);
		FntPrint(commandLines[index]);
		FntPrint("\n\n");
	}

	char szRamSize[ 40 ];
	sprintf( szRamSize, "_ramsize = %ld [%dMB]", _ramsize, _ramsize / (1024L * 1024L) );
	FntPrint( "\n" );
	FntPrint( szRamSize );
	FntPrint( "\n\n" );

	UpdateSimpleDisplay();

	PadInit(0);
	while(!(PadRead(0) & PADstart))
		;

	int commandLineIndex = (PadRead(0) >> 4) & 0xf;
	assert(strlen(commandLines[commandLineIndex]) < MAX_CMD_LINE);
	strcpy(inputBuffer,commandLines[commandLineIndex]);
	printf("command line = %s\n",inputBuffer);
	int fileSize = strlen(inputBuffer);
#else
	int fp = PCopen("velpsx.cmd", 0, 0);
	int fileSize = 0;
	if(fp != -1)
	 {
		fileSize = PClseek(fp, 0, 2);
		assert(fileSize > 0);
		AssertMsg(fileSize < MAX_CMD_LINE, "fileSize = " << fileSize << "  MAX_CMD_LINE = " << MAX_CMD_LINE );
		PClseek(fp, 0, 0);
		ASSERTIONS(int actual = ) PCread(fp, (char*)inputBuffer, fileSize);
		assert(actual == fileSize);
		PCclose(fp);
//		cout << "velpsx.cmd line = " << inputBuffer << endl;
	 }
	else
	 {
//		cout << "velpsx.cmd not found!" << endl;
//		exit(1);
	 }
	inputBuffer[fileSize] = '\0';									// zero terminate the input
#endif		// !DO_DEBUG_FILE_SYSTEM
#else		// HAL_SELECT_LEVEL
	char* inputBuffer = "0";
	int fileSize = 2;
#endif		// HAL_SELECT_LEVEL

	char* pLine = &inputBuffer[0];
	int argCount;
	argvPtrs[0] = "Game";					// set argv[0] to program name

	argCount = 1;
	while(*pLine )		// parse command line into arguments
	 {
		pLine = EatWhiteSpace(pLine);
		argvPtrs[argCount] = pLine;
		while(*pLine && *pLine != ' ' && *pLine != 0xa && *pLine != 0xd)
		 {
			assert(pLine < (&inputBuffer[0] + fileSize));			// insure we don't walk off end of file
			pLine++;
		 }
		if(*pLine)						// if not at end of inputBuffer, write a zero, then keep going
			*pLine++ = 0;
		pLine = EatWhiteSpace(pLine);

		assert(argCount < MAX_ARGC);

		// kts PIGS command line parsing
		if(!strcmp(argvPtrs[argCount],"-PIGSNoExceptionHandler"))
			installExceptionHandler = 0;
		else
			argCount++;
	 }

//	cout << "command count = " << argCount << endl;
	// end command line disection
	int argc = argCount;

	if(installExceptionHandler)
		_EX_Init();		// Install the Exception Handler

	char* * argv = argvPtrs;
	sys_init(&argc, &argv);

//	 this sets up all of the HAL, including the multi-tasker, and doesn't
//	 return until the  tasker shuts down
//	 note: these maximums are set in projlocl.mk, and at least MAX_TASKS is
//	 used elsewhere
	HALStart(argc, argv,HAL_MAX_TASKS,HAL_MAX_MESSAGES,HAL_MAX_PORTS);

	AssertMsg( 0, "End of HAL" );
	//NOTREACHED
	return 0;
}

//=============================================================================

struct ToT *tot = (struct ToT*)0x00000100;

//-----------------------------------------------------------------------------
// this routine does 3 things:
//  first it uses SetConf to allow more than 4 threads (see DTL H2000 docs)
//  then it loops through the available threads and sets their interrupt flags
//    correctly (see ReThread mesasge from Okamoto)
//  finally, it allocates threads 4,5,6,7 (numbered from 0), since the download
//    code overwrites this memory area (see Why5Thr message from Okamoto))

void
FixTCBBugs(int threadCount)
{
	DBSTREAM2( cprogress << "FixTCBBugs: " << endl; )
	assert(threadCount == HAL_MAX_TASKS);
	//long threadid[9];
	//unsigned long ev,tcba,sp;
    struct TCB *tcb;
    long i;

	EnterCriticalSection();
#if 1
// kts new bug fix code 10/10/96 10:05AM
	struct TCBH *headTCBH;
	struct TCB  *headTCB,*t0,*end;


	// allocate TCB area
	int size = sizeof(struct TCB) * threadCount;
//	headTCB = (struct TCB *)malloc(size);
	headTCB = new (*_HALLmalloc) TCB[threadCount];
	assert(headTCB);
	AssertMemoryAllocation(headTCB);

	// set TCB status
	for (t0 = headTCB,end = headTCB+threadCount;t0 < end;t0++)
		t0->status = TcbStUNUSED;
	headTCB->status =  TcbStACTIVE;

	// kts added: copy tcb at 0, since it is the master task
//	(*headTCB) = tcb_tab[0];

	// set current TCB
	headTCBH = (struct TCBH *) tot[1].head;
	headTCBH->entry = headTCB;

	// set TCB chain
	tot[2].head = (unsigned long *) headTCB;
	tot[2].size = size;
// end new code
#endif

    struct TCB *tcb_tab = (struct TCB*) (tot+2)->head;
	assert(tcb_tab == headTCB);

#if 0
// set enviroment to have more than 4 tasks
	GetConf(&ev, &tcba, &sp);
	SetConf(ev, threadCount, sp);
#endif

// enable interrupts for all tasks
    for(i=1;i<threadCount;i++)
	 {
            tcb = &tcb_tab[i];
            tcb->reg[R_SR] = 0x404;		// enable interrupt in interrupt context
     }
#if 0
// reserve tasks 4,5,6,7
	for(i=1;i<8;i++)				// allocate 1-7
	 {
		// create a few threads
		if( (threadid[i] = OpenTh(NULL,NULL,NULL)) == -1)
		assert(threadid[i] != -1);
	 }

	for(i=1;i<4;i++)				// now free 1,2 & 3
	 {
		CloseTh(threadid[i]);
	 }
#endif
	ExitCriticalSection();
}

//-----------------------------------------------------------------------------

// PAW 6/7/95 Initialize CD subsystem
#include <libcd.h>
#include <kernel.h>

//u_long _PigsRootCounter2;
//extern long _PigsRootCounterCallBack( void );

//-----------------------------------------------------------------------------

extern unsigned int __heapbase;
extern int __heapsize;

void
_PlatformSpecificInit( int /*argc*/, char* * /*argv*/, int /*maxTasks*/, int /*maxMessages*/, int /*maxPorts*/ )
{
	printf("ramsize = %d,heapbase = %p, heapsize = %d\n",_ramsize,__heapbase, __heapsize);
	if(__heapsize < 0)
		FatalError("heapsize too small, bad switch combination\n");
	char* membase = (char*)__heapbase;
	assert(ValidPtr(membase));

//	int memsize = __heapsize - ((membase+sizeof(LMalloc) - (char*)__heapbase));
	int memsize = __heapsize - sizeof(LMalloc);
	assert(ValidPtr(membase+memsize));
//	assert(memsize > 1000000);
//	assert(memsize < 8000000);
//	assert(memsize > 0);

	printf("HALLmalloc at address %p, size = %d\n",membase,memsize);
	_HALLmalloc = new (membase)
//		LMalloc( membase+sizeof(LMalloc), HALLMALLOC_SIZE MEMORY_NAMED( COMMA "HALLmalloc" ) );
		LMalloc( membase+sizeof(LMalloc), memsize MEMORY_NAMED( COMMA "HALLmalloc" ) );
	assert(ValidPtr(_HALLmalloc));
	_HALLmalloc->Validate();

extern DMalloc* _HALDmalloc;
	_HALDmalloc = new (*_HALLmalloc)
		DMalloc( (*_HALLmalloc), HAL_DMALLOC_SIZE MEMORY_NAMED( COMMA "HALDmalloc"));
	ValidatePtr(_HALDmalloc);

#define SCRATCH_SIZE 1024
	scratchPadMemory = new (*_HALLmalloc) LMalloc(getScratchAddr(0),SCRATCH_SIZE MEMORY_NAMED( COMMA "ScratchPad" )	);
	assert(ValidPtr(scratchPadMemory));
	InitHeap2( NULL, 0 );
	FixTCBBugs(HAL_MAX_TASKS);

	// kts moved cd init into dfcd.cc

	PadInit(0);						    // kts moved from psxjoy.c 1/15/96 1:34PM

//	 PAW 6/8/95
//	 Set up root counter 2
//	 42336 = 33,868,800 / 8 / 100 for 1/100th
//	 33705 = 33,868,800 / 8 / 128 for 1/128th of a second

// kts removed, since it is not accurate anyway
//	long e2 = OpenEvent( RCntCNT2, EvSpINT, EvMdINTR, _PigsRootCounterCallBack );
//	assert( e2 != -1L );
//	long retVal = SetRCnt( RCntCNT2, 33705, RCntMdINTR );
//	assert(retVal == 1);
//	retVal = EnableEvent( e2 );
//	assert(retVal == 1);
//	retVal = StartRCnt( RCntCNT2 );
//	assert(retVal == 1);
//	_PigsRootCounter2 = 0;
}

//=============================================================================

void
_PlatformSpecificUnInit()
{
	if ( installExceptionHandler )
		_EX_Quit();

#if defined( SOUND )
//	SsEnd();
//	SsQuit();

//    SpuSetKey( SpuOff, SPU_ALLCH );
//    SpuQuit();
#endif

    StopCallback();
}

//=============================================================================

bool _scratchPadInUse = false;

//=============================================================================
// debugging calls, used to determine the state of the psx os

#if DO_ASSERTIONS
#include <kernel.h>

char* totNames[32] =
{
	"Interrupt Queue",
	"Task State Queue Header",
	"Task Managment Block",
	"System Reserved",
	"Event Managment Block",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved",
	"System Reserved"
};

//-----------------------------------------------------------------------------

void
DumpToT(void)
{
	int i;

	printf("ToT Dump:\n");

	for(i=0;i<31;i++)
	{
		printf("  ToT Entry %2d, ptr=$%8p, length = $%8ld, name = %s\n",i,tot[i].head,tot[i].size,totNames[i]);
	}
}

#endif

//=============================================================================

void
FatalError( const char* string)
{
	InitSimpleDisplay();
	FntPrint( "\n\n -- Fatal Error -- \n%s\n",string);
	UpdateSimpleDisplay();


	while((PadRead(0) & PADstart))
		;
	while(1)
	{
		if(PadRead(0) == (PADL1|PADL2|PADR1|PADR2))
			return;
	}
}

//=============================================================================

extern "C"
{
#include <libsn.h>
}


void
dummy()
{
	assert(0);
	CdInit();
	CdControlB( CdlStop, 0, 0 );
 	CdSetDebug(2);		// CD debug level 2
	CdControlB( CdlStop, 0, 0 );
	CdSync( 0, 0 );
	CdControl( CdlSetmode, 0, 0 );
	CdSetDebug( 1 );		// CD debug level 2
	CdlLOC pos;
	CdIntToPos( 0, &pos );				// Same track as file
	CdControlB( CdlSetloc, (u_char*)NULL, 0 );	// Seek from the beginning of the file
	CdReadCallback( NULL );
	CdRead( 0, (u_long*)NULL, CdlModeSpeed );
	while ( CdReadSync( 1, 0 ) > 0 )
		;
}

//=============================================================================

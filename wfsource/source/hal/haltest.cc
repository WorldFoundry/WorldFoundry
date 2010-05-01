//=============================================================================
// test.c:
//=============================================================================

#include <hal/hal.h>
#include <hal/_list.h>
#include <hal/_mempool.h>

#if defined( __WIN__ )
#include <windows.h>
#endif

bool bShowWindow = false;

//=============================================================================
// kts put these here so as not to reveal them to clients  in the header files

#if TEST_GENERAL
void
GeneralTest(void);
#endif

#if TEST_TIMER
void
TimerTest(void);
#endif

#if DO_TEST_CODE
#if TEST_ITEM
void
ItemTest(void);
#endif
#endif

#if TEST_TASKER
void
TaskerTest(void);
#endif

#if TEST_DISKFILE
void
DiskFileTest(void);
#endif

//=============================================================================

void
TestHAL(void)
{
	printf("testing PIGS modules\n");
#if TEST_GENERAL
	printf("HAL Testing General\n");
	GeneralTest();
#endif
#if TEST_LIST
	printf("HAL Testing Lists\n");
	ListTest();
#endif
#if TEST_MEMPOOL
	printf("HAL Testing MemPools\n");
	MemPoolTest();
#endif

#if DO_TEST_CODE
#if TEST_ITEM
	printf("HAL Testing Items\n");
	ItemTest();
#endif
#endif
#if TEST_TASKER
	printf("HAL Testing Tasker\n");
	TaskerTest();
#endif
#if TEST_TIMER
	printf("HAL Testing Timer\n");
	TimerTest();
#endif

#if TEST_DISKFILE
	printf("HAL Testing DiskFile\n");
	DiskFileTest();
#endif
#if TEST_JOYSTICK
	printf("HAL Testing Joystick\n");
	JoystickTest();
#endif

#ifdef __PSX__
	// test mmgm malloc
	void *stock[100];
	long i;

//	printf("start of malloc\n");
//	for(i=0;i<20;i++)
//		if((stock[i]=malloc(0x8000))==NULL)
//			printf("%3d:allocation error\n",i);
//		else
//			printf("%3d:%08x\n",i,stock[i]);
//	printf("start of free\n");
//	for(i=0;i<20;i++)
//		if(stock[i]!=NULL)
//			free(stock[i]);
//	printf("end of test\n");
#endif

	printf("HAL Tests Complete\n");
}

//=============================================================================

void
PIGSMain(int argc, char* argv[])
{
//	malloc( 20 );
//	new char;
//
	printf("HAL Test Program ('%s', %d)\n", argv[0] ? argv[0] : "", argc);
	printf("argc = %d\n", argc);
	for(int index = 0;index < argc; index++)
	 {
		assert(argv[index]);
		printf("argv[%d] = %s\n",index, argv[index]);
	 }


extern void InitSimpleDisplay();
extern void UpdateSimpleDisplay();

#if 0
	InitSimpleDisplay();

	for(int i=0;i<100;i++)
		VSync(0);

	FntPrint("test 1\nmultiple lines");
	UpdateSimpleDisplay();
	FntPrint("test 2\nmultiple lines");
	UpdateSimpleDisplay();
	FntPrint("test 3\nmultiple lines");
	UpdateSimpleDisplay();
	FntPrint("test 4\nmultiple lines");
	UpdateSimpleDisplay();
	FntPrint("test 5\nmultiple lines");
	UpdateSimpleDisplay();
#endif

#if defined( __WIN__ )
	{ // Test Windows interface
//	extern HWND hwnd;
//	MessageBox( hwnd, "Message Box", "Title", MB_OK );
	}
#endif

//	_err_logbits = 0xf0;
	TestHAL();
	PIGSExit();
}

//=============================================================================

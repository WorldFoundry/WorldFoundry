//=============================================================================
// except.c:
//=============================================================================
// Exception handler for PSX.

// Ver		Date		Author			Desc
//----------------------------------------------------------------------------
// 0.1		09/08/95	Brian Marshall	Initial Version.
// 0.2		04-29-97 03:01pm Kevin Seghetti updated for PIGS

//=============================================================================

#include "except.h"

#include <cstdio>

#include <r3000.h>
#include <asm.h>
#include <kernel.h>
#include <cstddef>
#include <libsn.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

typedef struct tagEXCEPTIONDUMP
{
	DWORD rzero;
	DWORD rat;
	DWORD rv0;
	DWORD rv1;
	DWORD ra0;
	DWORD ra1;
	DWORD ra2;
	DWORD ra3;
	DWORD rt0;
	DWORD rt1;
	DWORD rt2;
	DWORD rt3;
	DWORD rt4;
	DWORD rt5;
	DWORD rt6;
	DWORD rt7;
	DWORD rs0;
	DWORD rs1;
	DWORD rs2;
	DWORD rs3;
	DWORD rs4;
	DWORD rs5;
	DWORD rs6;
	DWORD rs7;
	DWORD rt8;
	DWORD rt9;
	DWORD rk0;
	DWORD rk1;
	DWORD rgp;
	DWORD rsp;
	DWORD rfp;
	DWORD rra;
	DWORD rhi;
	DWORD rlo;
	DWORD rsr;
	DWORD rca;
	DWORD repc;
	DWORD badvaddr;
}EXCEPTIONDUMP;

extern EXCEPTIONDUMP Registers;

//static DWORD g_ExEvent;
//static DWORD g_ExEvent2;
//static DWORD g_ExEvent3;
//static DWORD g_ExEvent4;

//	Exception Types...

static char*	g_aExceptionTypes[] =
	{
		"External Interrupt.",
		"TLB Modification Exception.",
		"TLB Miss (Load or Fetch).",
		"TLB Miss (Store).",
		"Address Error Exception (Load or Fetch).",
		"Address Error Exception (Store).",
		"Bus Error (Fetch).",
		"Bus Error (Load or Store).",
		"Syscall.",
		"BreakPoint.",
		"Reserved Instruction.",
		"Coprocessor Unusable.",
		"Arithmetic Overflow.",
		"Unknown Exception.",
		"Unknown Exception.",
		"Unknown Exception."
	};

static DISPENV g_ExDispEnv;
static DRAWENV g_ExDrawEnv;

static char g_aExTemp[500];

//static void ExceptionHandler(void);

//	Function to hook the Vector. Call as early in your code as possible..

void
_EX_Init(void)
{
	InstallExceptionHandler();
}	// End _EX_Init

//	Exception Quit (Currently does nothing...)

void
_EX_Quit(void)
{
}	// End _EX_Quit

//	The Exception Handler Function.
//	Called by the Assembler when the Exception Happens.
//	The register state at the time of the exception in in the global
//	structure Registers.

void
CExceptionHandler(void)
{
	DWORD Cause;
	DWORD Pc;
	DWORD Type;
//	DWORD *pStack;
//	DWORD i;
	static RECT bg = {0, 0, 320, 240};

	printf("exception\n");

	ResetGraph(0);
	SetGraphDebug(0);
	FntLoad(640, 0);
	SetDumpFnt(FntOpen(0,12, 320, 200, 0, 1024));
	SetDefDrawEnv(&g_ExDrawEnv, 0, 0, 320, 240);
	SetDefDispEnv(&g_ExDispEnv, 0, 0, 320, 240);
	PutDrawEnv(&g_ExDrawEnv);
	PutDispEnv(&g_ExDispEnv);
	SetDispMask(1);

	Cause = Registers.rca;
	Pc = Registers.repc;
	Type = Cause;
	Type &= 0x1f;

	//	Check the Type....

	FntPrint("\nWorld Foundry Exception!\n");
	sprintf(g_aExTemp, "Type : %s\n\n", g_aExceptionTypes[Type]);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "PC : %08lx", Pc);
	FntPrint(g_aExTemp);
	if((Cause & 0x80000000) == 0x80000000)
	{
		FntPrint(" in Branch Delay Slot.");
	}

	sprintf(g_aExTemp,"\nBadVaddr = %08lx\n",Registers.badvaddr);
	FntPrint(g_aExTemp);

	FntPrint("\n\nRegisters:\n\n");
#if 1
	FntPrint("      00       01       02       03\n");
	sprintf(g_aExTemp, "0 :%08lx %08lx %08lx %08lx\n", Registers.rzero, Registers.rat, Registers.rv0, Registers.rv1);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "4 :%08lx %08lx %08lx %08lx\n", Registers.ra0, Registers.ra1, Registers.ra2, Registers.ra3);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "8 :%08lx %08lx %08lx %08lx\n", Registers.rt0, Registers.rt1, Registers.rt2, Registers.rt3);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "12:%08lx %08lx %08lx %08lx\n", Registers.rt4, Registers.rt5, Registers.rt6, Registers.rt7);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "16:%08lx %08lx %08lx %08lx\n", Registers.rs0, Registers.rs1, Registers.rs2, Registers.rs3);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "20:%08lx %08lx %08lx %08lx\n", Registers.rs4, Registers.rs5, Registers.rs6, Registers.rs7);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "24:%08lx %08lx xxxxxxxx %08lx\n", Registers.rt8, Registers.rt9, /*Registers.rk0,*/ Registers.rk1);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "28:%08lx %08lx %08lx %08lx\n", Registers.rgp, Registers.rsp, Registers.rfp, Registers.rra);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "\n  HI:%08lx LO:%08lx\n  SR:%08lx CA:%08lx\n", Registers.rhi, Registers.rlo, Registers.rsr, Registers.rca);
	FntPrint(g_aExTemp);
#else
	sprintf(g_aExTemp, "Z :%08lx AT:%08lx V0:%08lx V1:%08lx\n", Registers.rzero, Registers.rat, Registers.rv0, Registers.rv1);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "A0:%08lx A1:%08lx A2:%08lx A3:%08lx\n", Registers.ra0, Registers.ra1, Registers.ra2, Registers.ra3);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "T0:%08lx T1:%08lx T2:%08lx T3:%08lx\n", Registers.rt0, Registers.rt1, Registers.rt2, Registers.rt3);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "T4:%08lx T5:%08lx T6:%08lx T7:%08lx\n", Registers.rt4, Registers.rt5, Registers.rt6, Registers.rt7);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "S0:%08lx S1:%08lx S2:%08lx S3:%08lx\n", Registers.rs0, Registers.rs1, Registers.rs2, Registers.rs3);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "S4:%08lx S5:%08lx S6:%08lx S7:%08lx\n", Registers.rs4, Registers.rs5, Registers.rs6, Registers.rs7);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "T8:%08lx T9:%08lx K0:%08lx K1:%08lx\n", Registers.rt8, Registers.rt9, Registers.rk0, Registers.rk1);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "GP:%08lx SP:%08lx FP:%08lx RA:%08lx\n", Registers.rgp, Registers.rsp, Registers.rfp, Registers.rra);
	FntPrint(g_aExTemp);
	sprintf(g_aExTemp, "HI:%08lx LO:%08lx SR:%08lx CA:%08lx\n", Registers.rhi, Registers.rlo, Registers.rsr, Registers.rca);
	FntPrint(g_aExTemp);
#endif

	ClearImage(&bg, 10, 50,10);
	FntFlush(-1);
	DrawSync(0);
	VSync(0);
	SetDispMask(1);

die:
	goto die;
}	// End CExcpetionHandler

//=============================================================================

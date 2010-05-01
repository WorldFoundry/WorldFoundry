//=============================================================================
// gfx/psx/gteutil.cc: utilities for debugging gte code
// Copyright ( c ) 1997,99 World Foundry Group  
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
// Description:
//
// Original Author: Kevin T. Seghetti
//============================================================================

#include <pigsys\pigsys.hp>

#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <inline_c.h>

#if SW_DBSTREAM
//============================================================================

#define gte_StoreAllDataRegisters( r0 ) __asm__ volatile (			\
	"swc2	$0, 0( %0 );"					\
	"swc2	$1, 4( %0 );"					\
	"swc2	$2, 8( %0 );"					\
	"swc2	$3, 12( %0 );"					\
	"swc2	$4, 16( %0 );"					\
	"swc2	$5, 20( %0 );"					\
	"swc2	$6, 24( %0 );"					\
	"swc2	$7, 28( %0 );"					\
	"swc2	$8, 32( %0 );"					\
	"swc2	$9, 36( %0 );"					\
	"swc2	$10, 40( %0 );"					\
	"swc2	$11, 44( %0 );"					\
	"swc2	$12, 48( %0 );"					\
	"swc2	$13, 52( %0 );"					\
	"swc2	$14, 56( %0 );"					\
	"swc2	$15, 60( %0 );"					\
	"swc2	$16, 64( %0 );"					\
	"swc2	$17, 68( %0 );"					\
	"swc2	$18, 72( %0 );"					\
	"swc2	$19, 76( %0 );"					\
	"swc2	$20, 80( %0 );"					\
	"swc2	$21, 84( %0 );"					\
	"swc2	$22, 88( %0 );"					\
	"swc2	$23, 92( %0 );"					\
	"swc2	$24, 96( %0 );"					\
	"swc2	$25,100( %0 );"					\
	"swc2	$26,104( %0 );"					\
	"swc2	$27,108( %0 );"					\
	"swc2	$28,112( %0 );"					\
	"swc2	$29,116( %0 );"					\
	"swc2	$30,120( %0 );"					\
	"swc2	$31,124( %0 );"					\
	:							\
	: "r"( r0 )						\
	: "memory" )

#define gte_StoreAllControlRegisters( r0 ) __asm__ volatile (			\
	"cfc2	$12,$0;"						\
	"sw		$12, 0( %0 );"					\
	"cfc2	$12,$1;"						\
	"sw		$12, 4( %0 );"					\
	"cfc2	$12,$2;"						\
	"sw		$12, 8( %0 );"					\
	"cfc2	$12,$3;"						\
	"sw		$12, 12( %0 );"					\
	"cfc2	$12,$4;"						\
	"sw		$12, 16( %0 );"					\
	"cfc2	$12,$5;"						\
	"sw		$12, 20( %0 );"					\
	"cfc2	$12,$6;"						\
	"sw		$12, 24( %0 );"					\
	"cfc2	$12,$7;"						\
	"sw		$12, 28( %0 );"					\
	"cfc2	$12,$8;"						\
	"sw		$12, 32( %0 );"					\
	"cfc2	$12,$9;"						\
	"sw		$12, 36( %0 );"					\
	"cfc2	$12,$10;"						\
	"sw		$12, 40( %0 );"					\
	"cfc2	$12,$11;"						\
	"sw		$12, 44( %0 );"					\
	"cfc2	$12,$12;"						\
	"sw		$12, 48( %0 );"					\
	"cfc2	$12,$13;"						\
	"sw		$12, 52( %0 );"					\
	"cfc2	$12,$14;"						\
	"sw		$12, 56( %0 );"					\
	"cfc2	$12,$15;"						\
	"sw		$12, 60( %0 );"					\
	"cfc2	$12,$16;"						\
	"sw		$12, 64( %0 );"					\
	"cfc2	$12,$17;"						\
	"sw		$12, 68( %0 );"					\
	"cfc2	$12,$18;"						\
	"sw		$12, 72( %0 );"					\
	"cfc2	$12,$19;"						\
	"sw		$12, 76( %0 );"					\
	"cfc2	$12,$20;"						\
	"sw		$12, 80( %0 );"					\
	"cfc2	$12,$21;"						\
	"sw		$12, 84( %0 );"					\
	"cfc2	$12,$22;"						\
	"sw		$12, 88( %0 );"					\
	"cfc2	$12,$23;"						\
	"sw		$12, 92( %0 );"					\
	"cfc2	$12,$24;"						\
	"sw		$12, 96( %0 );"					\
	"cfc2	$12,$25;"						\
	"sw		$12,100( %0 );"					\
	"cfc2	$12,$26;"						\
	"sw		$12,104( %0 );"					\
	"cfc2	$12,$27;"						\
	"sw		$12,108( %0 );"					\
	"cfc2	$12,$28;"						\
	"sw		$12,112( %0 );"					\
	"cfc2	$12,$29;"						\
	"sw		$12,116( %0 );"					\
	"cfc2	$12,$30;"						\
	"sw		$12,120( %0 );"					\
	"cfc2	$12,$31;"						\
	"sw		$12,124( %0 );"					\
	:							\
	: "r"( r0 )						\
	: "memory" )



#define gte_LoadAllDataRegisters( r0 ) __asm__ volatile (			\
	"lwc2  $0 ,	 0( %0 );"					\
	"lwc2  $1 ,	 4( %0 );"					\
	"lwc2  $2 ,	 8( %0 );"					\
	"lwc2  $3 ,	 12( %0 );"					\
	"lwc2  $4 ,	 16( %0 );"					\
	"lwc2  $5 ,	 20( %0 );"					\
	"lwc2  $6 ,	 24( %0 );"					\
	"lwc2  $7 ,	 28( %0 );"					\
	"lwc2  $8 ,	 32( %0 );"					\
	"lwc2  $9 ,	 36( %0 );"					\
	"lwc2  $10,	 40( %0 );"					\
	"lwc2  $11,	 44( %0 );"					\
	"lwc2  $15,	 60( %0 );"					\
	"lwc2  $12,	 48( %0 );"					\
	"lwc2  $13,	 52( %0 );"					\
	"lwc2  $14,	 56( %0 );"					\
	"lwc2  $16,	 64( %0 );"					\
	"lwc2  $17,	 68( %0 );"					\
	"lwc2  $18,	 72( %0 );"					\
	"lwc2  $19,	 76( %0 );"					\
	"lwc2  $20,	 80( %0 );"					\
	"lwc2  $21,	 84( %0 );"					\
	"lwc2  $22,	 88( %0 );"					\
	"lwc2  $23,	 92( %0 );"					\
	"lwc2  $24,	 96( %0 );"					\
	"lwc2  $25,	100( %0 );"					\
	"lwc2  $26,	104( %0 );"					\
	"lwc2  $27,	108( %0 );"					\
	"lwc2  $28,	112( %0 );"					\
	"lwc2  $29,	116( %0 );"					\
	"lwc2  $30,	120( %0 );"					\
	"lwc2  $31,	124( %0 );"					\
	:							\
	: "r"( r0 ) )

#define gte_LoadAllControlRegisters( r0 ) __asm__ volatile (			\
	"lw	$12, 0( %0 );"					\
	"nop;"					\
	"ctc2	$12, $0;"					\
	"lw	$12, 4( %0 );"					\
	"nop;"					\
	"ctc2	$12, $1;"					\
	"lw	$12, 8( %0 );"					\
	"nop;"					\
	"ctc2	$12, $2;"					\
	"lw	$12, 12( %0 );"					\
	"nop;"					\
	"ctc2	$12, $3;"					\
	"lw	$12, 16( %0 );"					\
	"nop;"					\
	"ctc2	$12, $4;"					\
	"lw	$12, 20( %0 );"					\
	"nop;"					\
	"ctc2	$12, $5;"					\
	"lw	$12, 24( %0 );"					\
	"nop;"					\
	"ctc2	$12, $6;"					\
	"lw	$12, 28( %0 );"					\
	"nop;"					\
	"ctc2	$12, $7;"					\
	"lw	$12, 32( %0 );"					\
	"nop;"					\
	"ctc2	$12, $8;"					\
	"lw	$12, 36( %0 );"					\
	"nop;"					\
	"ctc2	$12, $9;"					\
	"lw	$12, 40( %0 );"					\
	"nop;"					\
	"ctc2	$12, $10;"					\
	"lw	$12, 44( %0 );"					\
	"nop;"					\
	"ctc2	$12, $11;"					\
	"lw	$12, 48( %0 );"					\
	"nop;"					\
	"ctc2	$12, $12;"					\
	"lw	$12, 52( %0 );"					\
	"nop;"					\
	"ctc2	$12, $13;"					\
	"lw	$12, 56( %0 );"					\
	"nop;"					\
	"ctc2	$12, $14;"					\
	"lw	$12, 60( %0 );"					\
	"nop;"					\
	"ctc2	$12, $15;"					\
	"lw	$12, 64( %0 );"					\
	"nop;"					\
	"ctc2	$12, $16;"					\
	"lw	$12, 68( %0 );"					\
	"nop;"					\
	"ctc2	$12, $17;"					\
	"lw	$12, 72( %0 );"					\
	"nop;"					\
	"ctc2	$12, $18;"					\
	"lw	$12, 76( %0 );"					\
	"nop;"					\
	"ctc2	$12, $19;"					\
	"lw	$12, 80( %0 );"					\
	"nop;"					\
	"ctc2	$12, $20;"					\
	"lw	$12, 84( %0 );"					\
	"nop;"					\
	"ctc2	$12, $21;"					\
	"lw	$12, 88( %0 );"					\
	"nop;"					\
	"ctc2	$12, $22;"					\
	"lw	$12, 92( %0 );"					\
	"nop;"					\
	"ctc2	$12, $23;"					\
	"lw	$12, 96( %0 );"					\
	"nop;"					\
	"ctc2	$12, $24;"					\
	"lw	$12, 100( %0 );"				\
	"nop;"					\
	"ctc2	$12, $25;"					\
	"lw	$12, 104( %0 );"				\
	"nop;"					\
	"ctc2	$12, $26;"					\
	"lw	$12, 108( %0 );"				\
	"nop;"					\
	"ctc2	$12, $27;"					\
	"lw	$12, 112( %0 );"				\
	"nop;"					\
	"ctc2	$12, $28;"					\
	"lw	$12, 116( %0 );"				\
	"nop;"					\
	"ctc2	$12, $29;"					\
	"lw	$12, 120( %0 );"				\
	"nop;"					\
	"ctc2	$12, $30;"					\
	"lw	$12, 124( %0 );"				\
	"nop;"					\
	"ctc2	$12, $31;"					\
	:							\
	: "r"( r0 )						\
	: "$12","$13" )

struct GTERegs
{
	int32 data[32];
	int32 control[32];
};

char* GTEControlNames[32] =
{
	"R11,R12   ",
	"R13,R21   ",
	"R22,R23   ",
	"R31,R32   ",
	"R33,      ",
	"TRX       ",
	"TRY       ",
	"TRZ       ",
	"L11,L12   ",
	"L13,L21   ",
	"L22,L23   ",
	"L31,L32   ",
	"L33,      ",
	"RBK       ",
	"GBK       ",
	"BBK       ",
	"LR1,LR2   ",
	"LR3,LG1   ",
	"LG2,LG3   ",
	"LB1,LB2   ",
	"LB3,      ",
	"RFC       ",
	"GFC       ",
	"BFC       ",
	"OFX       ",
	"OFY       ",
	"H         ",
	"DQA       ",
	"DQB       ",
	"ZSF3      ",
	"ZSF4      ",
	"FLAG      "
};

char* GTEDataNames[32] =
{
	"VX0,VY0   ",
	"VZ0       ",
	"VX1,VY1   ",
	"VZ1       ",
	"VX2,VY2   ",
	"VZ2       ",
	"RGB_CODE  ",
	"OTZ       ",
	"IR0       ",
	"IR1       ",
	"IR2       ",
	"IR3       ",
	"SX0,SY0   ",
	"SX1,SY1   ",
	"SX2,SY2   ",
	"SX2P,SY2P ",
	"SZx(0)    ",
	"SZ0(1)    ",
	"SZ1(2)    ",
	"SZ2(3)    ",
	"R0_G0_B0  ",
	"R1_G1_B1  ",
	"R2_G2_B2  ",
	"???       ",
	"MAC0      ",
	"MAC1      ",
	"MAC2      ",
	"MAC3      ",
	"IRGB      ",
	"ORGB      ",
	"DATA32    ",
	"LZC       "
};

//============================================================================

#if DO_DEBUG_FILE_SYSTEM

void
LoadGteRegs(ifstream& in)
{
	assert(in.good());
	GTERegs gteregs;
	char buffer[200];
	in.getline(buffer,200);
	assert(in.good());
//	cout << "line <" << buffer << ">" << endl;
	assert(!strcmp(buffer,"GTE Regs: Control              Data"));

	for(int index = 0; index < 32; index++)
	{
		in.getline(buffer,200);
		assert(in.good());
//			cout << "      " << setw(2) << index << ": " << setw(8) << gteregs.control[index] << " " << setw(8) << gteregs.data[index] << endl;
		char buffer2[200];
		char buffer3[200];
		int fileIndex = 1234;
//		cout << "line <" << buffer << ">" << endl;
		gteregs.control[index] = 0xdeaddead;
		gteregs.data[index] = 0xdeaddead;
		sscanf(buffer, "%2d:%s%8lx | %s%8lx", &fileIndex, buffer2,&gteregs.control[index], buffer3,&gteregs.data[index]);
		AssertMsg(index == fileIndex,"index = " << index << ", fileIndex = " << fileIndex);
//		cout << "buffer2 = <" << buffer2 << ">" << endl;
//		cout << "buffer3 = <" << buffer3 << ">" << endl;
//		AssertMsg(!strcmp(buffer2,GTEControlNames[index]),"buffer = <" << buffer2 << ">, controlnames = <" << GTEControlNames[index] << ">");
//		assert(!strcmp(buffer3,GTEDataNames[index]));

//		cout << "Read <" << buffer << "> got " << gteregs.control[index] << "," << gteregs.data[index] << endl;
	}
	gte_LoadAllDataRegisters(&gteregs.data);
	gte_LoadAllControlRegisters(&gteregs.control);
}

#endif
//============================================================================

void
PrintGTERegs(ostream& out)
{
	assert(out.good());
	GTERegs gteregs;
	gte_StoreAllDataRegisters(&gteregs.data);
	gte_StoreAllControlRegisters(&gteregs.control);
	out << "GTE Regs: Control              Data" << endl;
	for(int index = 0; index < 32; index++)
	{
//			cout << "      " << setw(2) << index << ": " << setw(8) << gteregs.control[index] << " " << setw(8) << gteregs.data[index] << endl;
//		printf("      %2d: %s%8lx | %s%8lx\n",index, GTEControlNames[index],gteregs.control[index], GTEDataNames[index],gteregs.data[index]);
		out <<  "      " << dec;
		out.width(2);
		out << index << ": " << GTEControlNames[index];
		out.width(8);
		out << hex << gteregs.control[index] << " | "  << GTEDataNames[index] << gteregs.data[index] << endl;
	}
}

//=============================================================================

void
DumpGTERegs()
{
	PrintGTERegs(cout);
}

#endif

//=============================================================================

#if DO_DEBUG_FILE_SYSTEM
#if SW_DBSTREAM >= 1

void
GTELab()
{
	PadInit(0);

	ResetGraph(0);
	SetGraphDebug(0);
	FntLoad(640, 0);
	SetDumpFnt(FntOpen(0,12, 320, 200, 0, 1024));
	static DRAWENV g_ExDrawEnv;
	SetDefDrawEnv(&g_ExDrawEnv, 0, 0, 320, 240);
	static DISPENV g_ExDispEnv;
	SetDefDispEnv(&g_ExDispEnv, 0, 0, 320, 240);
	PutDrawEnv(&g_ExDrawEnv);
	PutDispEnv(&g_ExDispEnv);

	static RECT bg = {0, 0, 320, 240};
	ClearImage(&bg, 10, 10,50);

	FntPrint("\nGTELab V0.2\n");
	FntPrint("<square  >: Load gte.txt\n");
	FntPrint("<circle  >: Save gte.txt\n");
	FntPrint("<triangle>: rtps\n");
	FntPrint("<X       >: nccs\n");

	FntFlush(-1);
	SetDispMask(1);

	InitGeom();

	cout << "GTELab:" << endl;
	cout << "<square  >: Load gte.txt" << endl;
	cout << "<circle  >: Save gte.txt" << endl;
	cout << "<triangle>: rtps" << endl;
	cout << "<X       >: ncds" << endl;

	while(1)
	{
		uint32 padd = PadRead(1);

		if(padd)
		{
			if (padd & PADRup)
			{
				cout << "rtps" << endl;
				gte_rtps();
			}

			if (padd & PADRdown)
			{
				cout << "ncds" << endl;
				gte_ncds();
			}
			if (padd & PADRleft)
			{
				cout << "Reading gte.in" << endl;
				ifstream in("gte.in");
				assert(in.good());
				LoadGteRegs(in);
			}
			if (padd & PADRright)
			{
				cout << "Writrting gte.out" << endl;
				ofstream out("gte.out");
				assert(out.good());
				PrintGTERegs(out);
			}
		//	if (padd & PADR1)
		//	if (padd & PADR2)

		//	if (padd & PADLup)
		//	if (padd & PADLdown)
		//	if (padd & PADLleft)
		//	if (padd & PADLright)

		//	if (padd & PADL1)
		//	if (padd & PADL2)

			while(PadRead(1))
				;			    // debounce
		}
	}
}

#endif
#endif
//============================================================================
// use joystick to look around video memory

void
ViewVideoMemory()
{
	DISPENV _dispEnv;
	int xPos = 0;
	int yPos = 0;

	while(PadRead(1) & PADL1);

	int cont = 1;
	while(cont)
	{
		uint32 padd = PadRead(1);


		if (padd & PADL1) 	cont = 0;

		int	xDelta = 10;
		int	yDelta = 10;
		// change the rotation angles for the cube and the light source
	//	if (padd & PADRup)
	//		rotation.SetA(rotation.GetA() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	//	if (padd & PADRdown)
	//		rotation.SetA(rotation.GetA() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	//	if (padd & PADRleft)
	//		rotation.SetB(rotation.GetB() - Angle(Angle::Degree(SCALAR_CONSTANT(1))));
	//	if (padd & PADRright)
	//		rotation.SetB(rotation.GetB() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));
		if (padd & PADR1)
		{
			xDelta = 1;
			yDelta = 1;
		}
	//	if (padd & PADR2)
	//		rotation.SetC(rotation.GetC() + Angle(Angle::Degree(SCALAR_CONSTANT(1))));

		if (padd & PADLup)
			yPos-= yDelta;
		if (padd & PADLdown)
			yPos += yDelta;
		if (padd & PADLleft)
			xPos -= xDelta;
		if (padd & PADLright)
			xPos += xDelta;

	//	if (padd & PADL1)
	//		position.SetZ(position.Z() - SCALAR_CONSTANT(0.5));
	//	if (padd & PADL2)
	//		position.SetZ(position.Z() + SCALAR_CONSTANT(0.5));


	SetDefDispEnv(&_dispEnv, xPos, yPos, 320, 240);
	PutDispEnv(&_dispEnv);
	SetDispMask(1);
	VSync(0);
	}
}


//============================================================================

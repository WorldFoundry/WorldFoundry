//==============================================================================
// disr3000.cc: Copyright (c) 1996-1999, World Foundry Group  
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


#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
using namespace std;

#include <cstdlib>
#include <cassert>
#include <cstdio>

#include "field.hp"
#include "mask.hp"
#include "fn.hp"
#include "patch.hp"

extern int currentSection;
extern vector<Patch> patches;

const char* immediate( unsigned short imm, bool bAlwaysShowSign=false );
const char* symbol( vector<Function> functions, unsigned long pc, unsigned long address, bool bOffset=true );

static Field field_op( 26, 31 );
static Field field_rs( 21, 25 );
static Field field_rt( 16, 20 );
static Field field_rd( 11, 15 );
static Field field_imm( 0, 15 );
static Field field_target( 0, 25 );
static Field field_shamt( 6, 10 );
static Field field_funct( 0, 5 );
static Field field_code( 6, 25 );
static Field field_cofun( 0, 24 );

char* szRegisterNames[ 32 ] = {
	"zero",
	"at",
	"v0",
	"v1",
	"a0",
	"a1",
	"a2",
	"a3",
	"t0",
	"t1",
	"t2",
	"t3",
	"t4",
	"t5",
	"t6",
	"t7",
	"s0",
	"s1",
	"s2",
	"s3",
	"s4",
	"s5",
	"s6",
	"s7",
	"t8",
	"t9",
	"k0",
	"k1",
	"gp",
	"sp",
	"fp",
	"ra"
	};


#define no_type()

#define j_type() \
	/*int op = field_op( *ptr );*/ \
	int target = field_target( *ptr ); \
	//printf( "\t%s\t%x,%x\n", __szOpcode__, op, target ); \

#define i_type() \
	/*int op = field_op( *ptr );*/ \
	int rs = field_rs( *ptr ); \
	int rt = field_rt( *ptr ); \
	int imm = field_imm( *ptr ); \
	//printf( "\t%s\t%x,%x,%x,%x\n", __szOpcode__, op, rs, rt, imm ); \

#define r_type() \
	/*int op = field_op( *ptr );*/ \
	int rs = field_rs( *ptr ); \
	int rt = field_rt( *ptr ); \
	int rd = field_rd( *ptr ); \
	int shamt = field_shamt( *ptr ); \
	int funct = field_funct( *ptr ); \
	//printf( "\t%s\t%x,%x,%x,%x,%x,%x\n", __szOpcode__, op, rs, rt, rd, shamt, funct ); \

#define code_type() \
	int code = field_code( *ptr );

#define cop_type() \
	int cofun = field_cofun( *ptr );


#define TEST_OPCODE( _opcodeMask, __type__, __print__ ) \
	else if ( mask##_opcodeMask( *ptr ) ) \
		{ \
		__type__##_type(); \
		out.setf( ios::left ); \
		out << setw(8) << setfill( ' ' ) << mask##_opcodeMask.name(); \
		out.unsetf( ios::left ); \
		out << hex << __print__ << endl; \
		} \


void
SWAP( char c1, char c2 )
{
	char _ = c1;
	c1 = c2;
	c2 = _;
}


signed long
offset( unsigned short imm )
	{
	return (signed short)imm << 2;
	}


const char*
immediate( unsigned short imm, bool bAlwaysShowSign )
	{
	static char _szBuffer[ 20 ];

	signed short limm = imm;
	if ( imm >= 0x8000 )
		{
		limm = -limm;
		sprintf( _szBuffer, "-$%x", limm );
		}
	else if ( imm == 0 )
		strcpy( _szBuffer, "0" );
	else
		{
		if ( bAlwaysShowSign )
			sprintf( _szBuffer, "+$%x", limm );
		else
			sprintf( _szBuffer, "$%x", limm );
		}

	return _szBuffer;
	}


const char*
symbol( vector<Function> functions, unsigned long pc, unsigned long address, bool bOffset )
{
	static char _szBuffer[ 128 ];

	char* sz = "\01_";
	Function functionToFind( sz, 0, currentSection, int( address ) );
	const Function* fn = find( functions.begin(), functions.end(), functionToFind );
	if ( fn != functions.end() )
		sprintf( _szBuffer, "%s ($%lx)", fn->name().c_str(), address );
	else
	{	// Look for closest symbol
		vector<Function>::iterator fn;
		vector<Function>::iterator targetAddr = NULL;
		for ( fn = functions.begin(); fn != functions.end(); ++fn )
		{
			if ( (fn->section() == currentSection) && ( fn->offset() < address ) )
			{
				if ( !targetAddr || ( fn->offset() > targetAddr->offset() ) )
				{
				targetAddr = fn;
				}
			}
		}
		//assert( targetAddr );
		if ( targetAddr )
		{
			int offsetFromFunction = address - targetAddr->offset();
			//assert( offsetFromFunction > 0 );
			if ( bOffset )
			{
				int offset = signed( address - pc );
				sprintf( _szBuffer, "$%lx (%s=%s+$%x)", address, immediate( offset, true ), targetAddr->name().c_str(),
					offsetFromFunction );
			}
			else
				sprintf( _szBuffer, "$%lx (=%s+$%x)", address, targetAddr->name().c_str(), offsetFromFunction );
		}
		else
			sprintf( _szBuffer, "$%lx", address );
	}

	return _szBuffer;
}


void
disassemble( void* data, size_t cbSize, unsigned long address, ostream& out, vector<Function> functions )
	{
	unsigned long* ptr = (unsigned long*)data;
//	assert( ( cbSize % 4 ) == 0 );
//	cbSize /= 4;

	Mask maskADD( "0000 00xx xxxx xxxx xxxx x000 0010 0000", "add" );
	Mask maskADDI( "001000 xxxxx xxxxx xxxxxxxxxxxxxxxx", "addi" );
	Mask maskADDIU( "001001 xxxxx xxxxx xxxxxxxxxxxxxxxx", "addiu" );
	Mask maskADDU( "000000 xxxxx xxxxx xxxxx 00000 100001", "addu" );
	Mask maskAND( "000000 xxxxx xxxxx xxxxx 00000 100100", "and" );
	Mask maskANDI( "001100 xxxxx xxxxx xxxxxxxxxxxxxxxx", "andi" );
	Mask maskBEQ( "0001 00xx xxxx xxxx xxxx xxxx xxxx xxxx", "beq" );
	Mask maskBGEZ( "0000 01xx xxx0 0001 xxxx xxxx xxxx xxxx", "bgez" );
	//BGEZAL
	Mask maskBGTZ( "0001 11xx xxx0 0000 xxxx xxxx xxxx xxxx", "bgtz" );
	Mask maskBLEZ( "0001 10xx xxx0 0000 xxxx xxxx xxxx xxxx", "blez" );
	Mask maskBLTZ( "0000 01xx xxx0 0000 xxxx xxxx xxxx xxxx", "bltz" );
	//BLTZAL
	Mask maskBNE( "0001 01xx xxxx xxxx xxxx xxxx xxxx xxxx", "bne" );
	Mask maskBREAK( "0000 00xx xxxx xxxx xxxx xxxx xx00 1101", "break" );
	Mask maskCOP0( "01000 01 x xxxx xxxx xxxx xxxx xxxx xxxx", "cop0" );
	Mask maskCOP1( "01000 11 x xxxx xxxx xxxx xxxx xxxx xxxx", "cop1" );
	Mask maskCOP2( "01001 01 x xxxx xxxx xxxx xxxx xxxx xxxx", "cop2" );
	Mask maskCOP3( "01001 11 x xxxx xxxx xxxx xxxx xxxx xxxx", "cop3" );
	Mask maskDIV( "000000 xxxxx xxxxx 00 0000 0000 011010", "div" );
	Mask maskDIVU( "000000 xxxxx xxxxx 00 0000 0000 011011", "divu" );
	Mask maskERET( "010000 1 000 0000 0000 0000 0000 011000", "eret" );
	Mask maskJ( "000010 xxxxxxxxxxxxxxxxxxxxxxxxxx", "j" );
	Mask maskJAL( "000011 xxxxxxxxxxxxxxxxxxxxxxxxxx", "jal" );
	Mask maskJALR( "000000 xxxxx 00000 xxxxx 00000 001001", "jalr" );
	Mask maskJR( "000000 xxxxx 000 0000 0000 0000 001000", "jr" );
	Mask maskLB( "100000 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lb" );
	Mask maskLBU( "100100 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lbu" );
	Mask maskLH( "100001 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lh" );
	Mask maskLHU( "100101 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lhu" );
	Mask maskLUI( "001111 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lui" );
	Mask maskLW( "100011 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lw" );
	//LWCz
	Mask maskLWL( "100010 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lwl" );
	Mask maskLWR( "100110 xxxxx xxxxx xxxxxxxxxxxxxxxx", "lwr" );
	Mask maskMFHI( "000000 00 0000 0000 xxxxx 00000 010000", "mfhi" );
	Mask maskMFLO( "000000 00 0000 0000 xxxxx 00000 010010", "mflo" );
	Mask maskMTHI( "000000 xxxxx 000 0000 0000 0000 010001", "mthi" );
	Mask maskMTLO( "000000 xxxxx 000 0000 0000 0000 010011", "mtlo" );
	Mask maskMULT( "000000 xxxxx xxxxx 00 0000 0000 011000", "mult" );
	Mask maskMULTU( "000000 xxxxx xxxxx 00 0000 0000 011001", "multu" );
	Mask maskNOR( "000000 xxxxx xxxxx xxxxx 00000 100111", "nor" );
	Mask maskOR( "000000 xxxxx xxxxx xxxxx 00000 100101", "or" );
	Mask maskORI( "001101 xxxxx xxxxx xxxxxxxxxxxxxxxx", "ori" );
	//RFE
	Mask maskSB( "101000 xxxxx xxxxx xxxxxxxxxxxxxxxx", "sb" );
	Mask maskSH( "101001 xxxxx xxxxx xxxxxxxxxxxxxxxx", "sh" );
	Mask maskSLL( "000000 xxxxx xxxxx xxxxx xxxxx 000000", "sll" );
	Mask maskSLLV( "000000 xxxxx xxxxx xxxxx 00000 000100", "sllv" );
	Mask maskSLT( "000000 xxxxx xxxxx xxxxx 00000 101010", "slt" );
	Mask maskSLTI( "001010 xxxxx xxxxx xxxxxxxxxxxxxxxx", "slti" );
	Mask maskSLTIU( "001011 xxxxx xxxxx xxxxxxxxxxxxxxxx", "sltiu" );
	Mask maskSLTU( "000000 xxxxx xxxxx xxxxx 00000 101011", "sltu" );
	Mask maskSRA( "000000 00000 xxxxx xxxxx xxxxx 000011", "sra" );
	Mask maskSRAV( "000000 xxxxx xxxxx xxxxx 00000 000111", "srav" );
	Mask maskSRL( "000000 xxxxx xxxxx xxxxx xxxxx 000010", "srl" );
	Mask maskSRLV( "000000 xxxxx xxxxx xxxxx 00000 000110", "srlv" );
	Mask maskSUB( "000000 xxxxx xxxxx xxxxx 00000 100010", "sub" );
	Mask maskSUBU( "000000 xxxxx xxxxx xxxxx 00000 100011", "subu" );
	Mask maskSW( "101011 xxxxx xxxxx xxxxxxxxxxxxxxxx", "sw" );
	Mask maskSWL( "101010 xxxxx xxxxx xxxxxxxxxxxxxxxx", "swl" );
	Mask maskSWR( "101110 xxxxx xxxxx xxxxxxxxxxxxxxxx", "swr" );
	Mask maskSYSCALL( "000000 xxxx xxxx xxxx xxxx xxxx 001100",  "syscall" );
	//TLBP, TLBR, TLBWI, TLBWR
	Mask maskXOR( "000000 xxxxx xxxxx xxxxx 00000 100110", "xor" );
	Mask maskXORI( "001110 xxxxx xxxxx xxxxxxxxxxxxxxxx", "xori" );

	for ( int i=0; i<cbSize; i+=sizeof(long), ++ptr, address+=sizeof(long) )
		{
		char* sz = "\01_";
		Function toFind( sz, 0, currentSection, int( address ) );
		const Function* fn = find( functions.begin(), functions.end(), toFind );
		if ( fn != functions.end() )
			cout << endl << fn->name() << ':' << endl;

		{
		unsigned long data = *ptr;
		char* p = (char*)&data;
		SWAP( p[0], p[3] );
		SWAP( p[1], p[2] );
		*ptr = data;
		}

		out.setf( ios::uppercase );
		out << hex << setfill('0') << setw(8) << address << ' ' << setw(8) << *ptr << "          ";
		out.unsetf( ios::uppercase );

		if ( *ptr == 0 )
			{
			out << "nop" << endl;
			}
		TEST_OPCODE( ADD, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		else if ( maskADDU( *ptr ) )
			{ // TEST_OPCODE( ADDU, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
			r_type();
			if ( rt == 0 )
				{
				out.setf( ios::left );
				out << setw(8) << setfill( ' ' ) << "move";
				out.unsetf( ios::left );
				out << hex << szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << endl;
				}
			else
				{
				out.setf( ios::left );
				out << setw(8) << setfill( ' ' ) << maskADDU.name();
				out.unsetf( ios::left );
				out << hex << szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] << endl;
				}
			}
		TEST_OPCODE( AND, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( DIV, r, szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( DIVU, r, szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( JR, r, szRegisterNames[ rs ] )

		TEST_OPCODE( MULT, r, szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( MULTU, r, szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )

		TEST_OPCODE( OR, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( NOR, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( SLL, r, szRegisterNames[ rd ] << ',' << szRegisterNames[ rt ] << ',' << immediate( shamt ) )
		TEST_OPCODE( SLLV, r, szRegisterNames[ rd ] << ',' << szRegisterNames[ rt ] << ',' << szRegisterNames[ rs ] )
		TEST_OPCODE( SRA, r, szRegisterNames[ rd ] << ',' << szRegisterNames[ rt ] << ',' << immediate( shamt ) )
		TEST_OPCODE( SRAV, r, szRegisterNames[ rd ] << ',' << szRegisterNames[ rt ] << ',' << szRegisterNames[ rs ] )
		TEST_OPCODE( SRL, r, szRegisterNames[ rd ] << ',' << szRegisterNames[ rt ] << ',' << immediate( shamt ) )
		TEST_OPCODE( SRLV, r, szRegisterNames[ rd ] << ',' << szRegisterNames[ rt ] << ',' << szRegisterNames[ rs ] )
		TEST_OPCODE( SUB, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( SUBU, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( XOR, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( SLT, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )
		TEST_OPCODE( SLTU, r, szRegisterNames[ rd ] << "," << szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] )

		TEST_OPCODE( ADDI, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << "," << setfill('0') << immediate( imm ) )
		TEST_OPCODE( ANDI, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << ",$" << setfill('0') << setw(4) << imm )
		TEST_OPCODE( ORI, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << ",$" << setfill('0') << setw(4) << imm )
		TEST_OPCODE( XORI, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << ",$" << setfill('0') << setw(4) << imm )
		TEST_OPCODE( ADDIU, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << "," << setfill('0') << immediate( imm ) )
		TEST_OPCODE( SLTI, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << ',' << setfill('0') << immediate( imm ) )
		TEST_OPCODE( SLTIU, i, szRegisterNames[ rt ] << "," << szRegisterNames[ rs ] << "," << setfill('0') << immediate( imm ) )
		TEST_OPCODE( LUI, i, szRegisterNames[ rt ] << ",$" << setfill('0') << setw(4) << imm )

		TEST_OPCODE( BEQ, i,  szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] << "," << symbol( functions, address, address + offset( imm+1 ) ) )
		TEST_OPCODE( BNE, i, szRegisterNames[ rs ] << ',' << szRegisterNames[ rt ] << "," << symbol( functions, address, address + offset( imm+1 ) ) )
		TEST_OPCODE( BGEZ, i, szRegisterNames[ rs ] << "," << symbol( functions, address, address + offset( imm+1 ) ) )
		TEST_OPCODE( BGTZ, i, szRegisterNames[ rs ] << "," << symbol( functions, address, address + offset( imm+1 ) ) )
		TEST_OPCODE( BLEZ, i, szRegisterNames[ rs ] << "," << symbol( functions, address, address + offset( imm+1 ) ) )
		TEST_OPCODE( BLTZ, i, szRegisterNames[ rs ] << "," << symbol( functions, address, address + offset( imm+1 ) ) )
		TEST_OPCODE( J, j, symbol( functions, address, ( (address & 0xF0000000) | (target<<2) ), false ) )
		TEST_OPCODE( JAL, j, symbol( functions, address, ( (address & 0xF0000000) | (target<<2) ), false ) )

		TEST_OPCODE( LB, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( LBU, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( LH, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( LHU, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( LW, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( SB, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( SH, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( SW, i, szRegisterNames[ rt ] << "," << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )

		TEST_OPCODE( MFHI, r, szRegisterNames[ rd ] )
		TEST_OPCODE( MFLO, r, szRegisterNames[ rd ] )
		TEST_OPCODE( MTHI, r, szRegisterNames[ rs ] )
		TEST_OPCODE( MTLO, r, szRegisterNames[ rs ] )

		TEST_OPCODE( LWL, i, szRegisterNames[ rt ] << ',' << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( LWR, i, szRegisterNames[ rt ] << ',' << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( SWL, i, szRegisterNames[ rt ] << ',' << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )
		TEST_OPCODE( SWR, i, szRegisterNames[ rt ] << ',' << immediate( imm ) << '(' << szRegisterNames[ rs ] << ')' )

		TEST_OPCODE( COP0, cop, '$' << cofun )
		TEST_OPCODE( COP1, cop, '$' << cofun )
		TEST_OPCODE( COP2, cop, '$' << cofun )
		TEST_OPCODE( COP3, cop, '$' << cofun )

		TEST_OPCODE( ERET, no, "" )

		else if ( maskJALR( *ptr ) )
			{ // TEST_OPCODE( JALR, r, rd==31 ? (szRegisterNames[ rs ]) : (szRegisterNames[ rd ] << ',' << szRegisterNames[ rs ]) )
			r_type();
			out.setf( ios::left );
			out << setw(8) << setfill( ' ' ) << maskJALR.name();
			out.unsetf( ios::left );
			if ( rd == 31 )
				out << hex << szRegisterNames[ rs ] << endl;
			else
				out << hex << szRegisterNames[ rd ] << ',' << szRegisterNames[ rs ] << endl;
			}

		TEST_OPCODE( BREAK, code, '$' << code )
		TEST_OPCODE( SYSCALL, code, '$' << code )

		else
			{
			out.setf( ios::left );
			out << setw(8) << setfill( ' ' ) << "dl";
			out.unsetf( ios::left );
			out << hex << '$' << *ptr << endl;
			}
		out.unsetf( ios::left );
		}
	}

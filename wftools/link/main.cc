//==============================================================================
// main.cc for disr3000
// By William B. Norris IV
// Copyright (c) 1997-1999, World Foundry Group  
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
// v0.0.1	?? Mar 97	Created
//==============================================================================

#include <iostream.h>
#include <fstream.h>
#include <stdlib.h>
#include <pigsys/assert.hp>
#include <string.h>
#include <stdio.h>
//#include <bool>

void usage( int argc, char* argv[] );
void disassemble( void*, size_t, unsigned long, ostream& );
//#include "profile.hp"

//#include "memcheck.h"
//MC_SET_ERF( erf_find_leaks );

ofstream* log;

bool bDebug = false;
bool bVerbose = false;

extern char szVersion[];


// Command line switches
const char verbose[] = "-verbose";
const char debugflag[] = "-debug";
const char szAddress[] = "-addr=";

void
usage( int argc, char* argv[] )
{
	cout << "disr3000 v" << szVersion << " Copyright 1997-1999 World Foundry Group." << endl;
	cout << "by William B. Norris IV" << endl;
	cout << "R3000 disassembler" << endl;
	cout << endl;
	cout << "Usage: disr3000 [switches] file" << endl;
	cout << '\t' << szAddress << "\t\tDisassemble at specified address" << endl;
	cout << '\t' << verbose << "\tEnable verbose messages" << endl;
}


int
main( int argc, char* argv[] )
{
	mc_startcheck( NULL );

	unsigned long address = 0UL;

	log = new ofstream( "link3000.log" );
	assert( log );

	char szConfig[] = "Configuration";

	char* szInputFilename = NULL;
	char* szOutputFilename = NULL;

	for ( ++argv; *argv; ++argv )
 	{
		if ( strcmp( *argv, "-?" ) == 0 )
 		{
			usage( argc, argv );
			return -1;
 		}
		else if ( strcmp( *argv, debugflag ) == 0 )
			bDebug = true;
		else if ( strcmp( *argv, verbose ) == 0 )
			bVerbose = true;
		else if ( strncmp( *argv, szAddress, strlen( szAddress ) ) == 0 )
			sscanf( *argv + strlen( szAddress ), "%lx", &address );
		else
 		{
			if ( !szInputFilename )
				szInputFilename = *argv;
			else
				szOutputFilename = *argv;
 		}
 	}

	if ( !szInputFilename )
 	{
		usage( argc, argv );
		return -1;
 	}

	ostream* out;
	if ( szOutputFilename )
		out = new ofstream( szOutputFilename, ios::text );
	else
		out = &cout;
	assert( out );

	{
	void* LoadFile( const char* _szFilename, size_t& nSize );

	size_t cbSize;
	void* ptr = LoadFile( szInputFilename, cbSize );
	disassemble( ptr, cbSize, address, *out );
	free( ptr );
	}

	delete log;

	delete out;

	mc_endcheck();

	return 0;
 }

//==============================================================================
// main.cc for stripper
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
// v0.1.0	24 Oct 97	Created
//==============================================================================

#include <iostream>
#include <fstream>
#include <cassert>
using namespace std;

#include <pigsys/assert.hp>

#include "stripper.hp"

ofstream* log;

char szOutFile[ _MAX_PATH ] = "out.out";
char* szIniFile;
char szOutDir[ _MAX_PATH ] = ".";

bool bDebug = false;
bool bVerbose = false;
bool bFrame = false;
TargetSystem ts = TARGET_WINDOWS;
bool bSourceControl = false;
Color clrBackground( Color::black );
int windowWidth = 640;
int windowHeight = 480;

extern char szVersion[];

// Command line switches
const char verbose[] = "-verbose";
const char debugflag[] = "-debug";
const char output[] = "-o=";
const char outdir[] = "-outdir=";
const char systemplaystation[] = "-Tpsx";
const char systemwindows[] = "-Twin";
//const char systemsaturn[] = "-Tsat";
//const char systemdos[] = "-Tdos";
const char sourcecontrol[] = "-sourcecontrol";
const char width[] = "-width=";
const char height[] = "-height=";
const char background[] = "-bg=";

void
usage( int argc, char* argv[] )
{
	cout << "stripper v" << szVersion << " Copyright 1997-1999 World Foundry Group." << endl;
	cout << "by William B. Norris IV" << endl;
	cout << "[ Description ]" << endl;
	cout << endl;
	cout << "Usage: stripper [switches] ???  []=default value" << endl;
	cout << '\t' << outdir << "\tOutput directory [" << szOutDir << ']' << endl;
	cout << '\t' << background << "\t\tSet background color (R,G,B) [60,60,60]" << endl;
	cout << '\t' << width << "\t\tWindow width [" << windowWidth << ']' << endl;
	cout << '\t' << height << "\tWindow height [" << windowHeight << ']' << endl;
	cout << '\t' << systemplaystation << "\t\tTarget system Playstation" << endl;
//	cout << '\t' << systemsaturn << "\t\tTarget system Saturn" << endl;
//	cout << '\t' << systemdos << "\t\tTarget system DOS4G" << endl;
	cout << '\t' << systemwindows << "\t\t[Target system Windows 95/Windows NT]" << endl;
	cout << '\t' << output << "\t\tOutput data file [" << szOutFile << ']' << endl;
	cout << '\t' << verbose << "\tEnable verbose messages [" << bVerbose << ']' << endl;
	cout << "[ Currently unimplemented, but planned ]:" << endl;
	cout << '\t' << sourcecontrol << "\tVerify input files are in revision control system [" << bSourceControl << ']' << endl;
}


int
main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		usage( argc, argv );
		return -1;
	}

	//mc_startcheck( NULL );

	log = new ofstream( "stripper.log" );
	assert( log );

	char szConfig[] = "Configuration";

	for ( ++argv; *argv; ++argv )
	{
		if ( strcmp( *argv, "-?" ) == 0 )
		{
			usage( argc, argv );
			return -1;
		}
		else if ( strcmp( *argv, systemplaystation ) == 0 )
			ts = TARGET_PLAYSTATION;
//		else if ( strcmp( *argv, systemsaturn ) == 0 )
//			ts = TARGET_SATURN;
//		else if ( strcmp( *argv, systemdos ) == 0 )
//			ts = TARGET_DOS;
		else if ( strcmp( *argv, systemwindows ) == 0 )
			ts = TARGET_WINDOWS;
		else if ( strcmp( *argv, debugflag ) == 0 )
			bDebug = true;
		else if ( strcmp( *argv, verbose ) == 0 )
			bVerbose = true;
		else if ( strncmp( *argv, outdir, strlen( outdir ) ) == 0 )
			strcpy( szOutDir, *argv + strlen( outdir ) );
		else if ( strcmp( *argv, sourcecontrol ) == 0 )
			bSourceControl = true;
		else if ( strncmp( *argv, output, strlen( output ) ) == 0 )
			strcpy( szOutFile, *argv + strlen( output ) );
		else if ( strncmp( *argv, background, strlen( background ) ) == 0 )
		{
			int r, g, b;
			sscanf( *argv + strlen( background ), "%d,%d,%d", &r, &g, &b );
			clrBackground = Color( r, g, b );
		}
		else if ( strncmp( *argv, width, strlen( width ) ) == 0 )
			windowWidth = atoi( *argv + strlen( width ) );
		else if ( strncmp( *argv, height, strlen( height ) ) == 0 )
			windowHeight = atoi( *argv + strlen( height ) );
		else
		{
			stripper( *argv );
		}
#if 0
		else
		{
			cerr << "Unknown command line option " << *argv << endl;
			return -1;
		}
#endif
	}

	delete log;

	//mc_endcheck();

	return 0;
}

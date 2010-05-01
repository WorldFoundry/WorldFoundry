//==============================================================================
// main.cc for psxprof
// By William B. Norris IV
// Copyright (c) 1998-1999, World Foundry Group  
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
//==============================================================================
// v0.1.0	01 Jul 96	Started
// v0.2.0   13 Jul 98	Compiles under Visual C++
//==============================================================================

#include <iostream>
#include <fstream>
using namespace std;
#include <cassert>
#include <version/version.hp>

void usage( int argc, char* argv[] );

ofstream* log;

void psxprof( const char*, const char* );

const char szCutOff[] = "-cutoff=";
double cutoffPercentage = 0.1;
const char szBarGraph[] = "-nobargraph";
bool bBarGraph = true;
const char szText[] = "-text";
const char szHtml[] = "-html";
bool bHtml = true;
const char szRelative[] = "-relative";
const char szAbsolute[] = "-absolute";
bool bRelative = true;

void
usage( int argc, char* argv[] )
{
	cout << "psxprof v" << szVersion << " Copyright 1996,1998,99 World Foundry Group." << endl;
	cout << "by William B. Norris IV" << endl;
	cout << "PlayStation program sampling analysis" << endl;
	cout << endl;
	cout << "Usage: psxprof mapFile sampleFile" << endl;
	cout << '\t' << szCutOff << "\tDon't show percentages below percentage [" << cutoffPercentage << "%]" << endl;
	cout << '\t' << szBarGraph << "\tDon't show bar graph" << endl;
	cout << '\t' << szHtml << "\t\tGenerate report as HTML [default]" << endl;
	cout << '\t' << szText << "\t\tGenerate report as text" << endl;
	cout << '\t' << szRelative << "\tBar graphs relative to largest time [default]" << endl;
	cout << '\t' << szAbsolute << "\tBar graphs relative to the total time of 100%" << endl;
}


int
main( int argc, char* argv[] )
{
	if ( argc == 1 )
	{
		usage( argc, argv );
		return -1;
	}

	log = new ofstream( "psxprof.log" );
	assert( log );

	for ( ++argv; *argv; ++argv )
	{
		if ( strcmp( *argv, "-?" ) == 0 )
		{
			usage( argc, argv );
			return -1;
		}
		else if ( strncmp( *argv, szCutOff, strlen( szCutOff ) ) == 0 )
			cutoffPercentage = atof( *argv + strlen( szCutOff ) ) / 100.0;
		else if ( strncmp( *argv, szBarGraph, strlen( szBarGraph ) ) == 0 )
			bBarGraph = false;
		else if ( strcmp( *argv, szHtml ) == 0 )
			bHtml = true;
		else if ( strcmp( *argv, szText ) == 0 )
			bHtml = false;
		else if ( strcmp( *argv, szRelative ) == 0 )
			bRelative = true;
		else if ( strcmp( *argv, szAbsolute ) == 0 )
			bRelative = false;
		else
		{
			psxprof( *argv, *(argv+1) );
			delete log;
			return 0;
		}
	}

	delete log;
	return 0;
}

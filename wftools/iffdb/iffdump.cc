//=============================================================================
// iffdump.cc: kts iff reader, creates an in-memory database of an iff stream
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

#include "global.hp"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <recolib/command.hp>
#include <recolib/hdump.hp>
#include <cpplib/stdstrm.hp>
#include <streams/binstrm.hp>
#include <iff/iffread.hp>
#include "iffdb.hp"
#include "chunklist.hp"
#include "indent.hp"

#include <map>
using namespace std;

#if DEBUG
#define DEBUG_STRING "debug"
#else
#define DEBUG_STRING "release"
#endif

#define VERSION_STRING "0.04"

//=============================================================================

ostream* output;
int charsPerLine=16;
bool dumpBinary=true;
bool useHDump=true;
string chunkFileName("chunks.txt");

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(const CommandLine& cl)
{
	for ( int commandIndex=1; commandIndex < cl.Size() && (cl[commandIndex][0] == '-'); ++commandIndex )
	{
		string command = cl[commandIndex];
		assert(command[0] == '-');
		DBSTREAM2( cdebug << "command line option <" << command << "> Found" << endl; )
		string restOfCommand = string(command.substr(2,command.length()-2));
		switch(command[1])
		{
			case 'c':
			case 'C':
				chunkFileName = restOfCommand;
				break;
			case 'f':
			case 'F':
				useHDump = command[2] == '+'?true:false;
				break;
			case 'd':
			case 'D':
				dumpBinary = command[2] == '+'?true:false;
				break;
			case 'w':
			case 'W':
				assert(command[2] == '=');
				charsPerLine = atoi((string(command.substr(3,command.length()-2))).c_str());
				assert(charsPerLine > 0);
				break;
			case 'p':
			case 'P':
				RedirectStandardStream(restOfCommand.c_str());
				break;
			default:
				cerror << "iffdump Error: Unrecognized command line switch \"" <<
					command << "\"" << endl;
				break;
		 }
	 }

	if(cl.Size()-(commandIndex-1) < 2)
	{
		cout << "iffdump V" VERSION_STRING " " DEBUG_STRING " Copyright 1998,99 World Foundry Group." << endl;
		cout << "Usage: iffdump {-<switches>} <infile> {<outfile>}" << endl;
		cout << "Switches:" << endl;
		cout << "    -d<filename> override default chunk file" << endl;
		cout << "    -d<+|-> enable/disable dump binary" << endl;
		cout << "    -f<+|-> enable hdump vs iffcomp compatible hdump" << endl;
		cout << "    -w=<#> # of chars per line in binary dump" << endl;
		cout << "    -p<stream initial><stream output>, where:" << endl;
		cout << "        <stream initial> can be any of:" << endl;
		cout << "            w=warnings (defaults to standard err)" << endl;
		cout << "            e=errors (defaults to standard err)" << endl;
		cout << "            f=fatal (defaults to standard err)" << endl;
		cout << "            s=statistics (defaults to null)" << endl;
		cout << "            p=progress (defaults to null)" << endl;
		cout << "            d=debugging  (defaults to null)" << endl;
		cout << "        <stream output> can be any of:" << endl;
		cout << "            n=null, no output is produced" << endl;
		cout << "            s=standard out" << endl;
		cout << "            e=standard err" << endl;
		cout << "            f<filename>=output to filename" << endl;
		exit(1);
	}
	return(commandIndex);
}

//=============================================================================

int
main(int argc, char** argv)
{
	CommandLine cl(argc,argv);

	output = &cout;

	int commandIndex = ParseCommandLine(cl);
//	cout << "commandIndex = " << commandIndex << endl;

	cdebug << "iffdump Debugging stream active!" << endl;

#pragma message(__FILE__ ": kts: temporary, make a binistream that reads from disk")

	binistream input(cl[commandIndex].c_str());
	assert(input.good());
	cstats << "Reading from source file " << cl[commandIndex] << endl;

	ifstream chunkNames(chunkFileName.c_str());
	ChunkList* chunkList = new ChunkList(chunkNames);
	if(cl.Size() > 1+commandIndex)
	{
		output = new ofstream(cl[1+commandIndex].c_str());
		if(output->rdstate() != ios::goodbit)
	  	{
			cerror << "Unable to open output file " << cl[1+commandIndex] << endl;
			return 5;
	  	}
		cstats << "Writing to dest file " << cl[1+commandIndex] << endl;
	}
	ostream& out = *output;

	out << "//=============================================================================" << endl;
	out << "// ";
	if(cl.Size() > 1+commandIndex)
		out << cl[1+commandIndex];
	out << " Created by IFF DUMP " VERSION_STRING " DO NOT MODIFY" << endl;
	out << "//=============================================================================" << endl;

	IFFDataBase db(input,*chunkList);
	out << db << endl;

	delete output;
	delete chunkList;
	return 0;
}

//=============================================================================

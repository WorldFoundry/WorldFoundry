//=============================================================================
// prep.cc: 
// By Kevin T. Seghetti
// Copyright (c) 1995-1999, World Foundry Group  
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
#include <stdlib.h>
#include <signal.h>

#if USE_OLD_IOSTREAM
#include <iostream.h>
#include <fstream.h>
#include <map.h>
#else
#include <iostream>
#include <fstream>
#include <map>
using namespace std;
#endif

#include <cpplib/stdstrm.hp>
#include "source.hp"
#include <recolib/ktstoken.hp>
#include "prep.hp"
#include <recolib/command.hp>

#if DEBUG
#define DEBUG_STRING "debug"
#else
#define DEBUG_STRING "release"
#endif

//=============================================================================

map<string,macro,less<string> > commandMacros;

//=============================================================================

const char whiteChars[] = " \t\n";
const char delimiters[] = " \t\n{}(),+-*/=%|^$[]\\!~:;\"'(@";			// note: delimiters must be a super-set of whiteChars

//=============================================================================
// global version of SourceError, until I get a better idea

string
SourceError()
{
	return(prepInput->SourceError());
}

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(const CommandLine& cl)
{
    unsigned int commandIndex;
	for ( commandIndex=1; commandIndex < cl.Size() && (cl[commandIndex][0] == '-'); ++commandIndex )
	{
		string command = cl[commandIndex];
		assert(command[0] == '-');
		DBSTREAM2( cdebug << "command line option <" << command << "> Found" << endl; )
		string restOfCommand = string(command.substr(2,command.length()-2));
		switch(command[1])
		{
#if DEBUG
			case 'p':
			case 'P':
				RedirectStandardStream(restOfCommand.c_str());
				break;
#endif
			case 'd':
			case 'D':
			{
				ktsRWCTokenizer next(restOfCommand);
				string token = next("\n="," \t\n");
				DBSTREAM2( cdebug << "Defining new macro <" << token << ">" << endl; )
				if(next.MatchNextChar("="))
				{
					next+=1;		// skip = sign
					commandMacros[token] = macro(token,next(),string(""),string("Defined on the command line"));
				}
				else
				{
					cerror << "Prep Error: Command Line error in defining macro, = not found" << endl;
					exit(1);
				}
				break;
			}

			default:
				cerror << "Prep Error: Unrecognized command line switch \"" <<
					command << "\"" << endl;
				break;
		 }
	 }

	if(cl.Size()-(commandIndex-1) < 2)
	{
                cout << "Prep V0.103 " DEBUG_STRING " Copyright 1995-2003 Kevin T. Seghetti." << endl;
		cout << "Usage: prep {-<switches>}<infile> {<outfile>}" << endl;
		cout << "Switches:" << endl;
		cout << "    -d<macro name>=<macro string>" << endl;
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

source* prepInput;
ostream* prepOutput;

//=============================================================================
// ctrl-break handler

static void
restore_cwd( int )
{
//	_dos_setdrive( drive, &_ );
//	chdir( cwd );
//	if(prepOutput)
//	 {
//
//	 }
	exit( 1 );
}

//=============================================================================

int
main(int argc, char** argv)
{
	CommandLine cl(argc,argv);
//	cout << cl << endl;
    // kts make source able to take an input stream
	//prepInput = &cin;
	prepOutput = &cout;

	signal( SIGINT, restore_cwd );				// set up ctrl-break handler

	unsigned int commandIndex = ParseCommandLine(cl);
//	cout << "commandIndex = " << commandIndex << endl;

	DBSTREAM1(cdebug << "Prep Debugging stream active!" << endl; )

	//if(cl.Size() > commandIndex)
    //{
	    prepInput = new source(cl[commandIndex],commandMacros);
	    DBSTREAM1( cstats << "Reading from source file " << cl[commandIndex] << endl; )
    //}

	if(cl.Size() > 1+commandIndex)
	{
		prepOutput = new ofstream(cl[1+commandIndex].c_str());
	if(prepOutput->rdstate() != ios::goodbit)
	  	{
			cerror << "Unable to open output file " << cl[1+commandIndex] << endl;
			return 5;
	  	}
		DBSTREAM1( cstats << "Writing to destination file " << cl[1+commandIndex] << endl; )
	}

	while(prepInput->LinesLeft())
	{
		string line;
		line = prepInput->GetNextLine();
		(*prepOutput) << line << endl;
	}

    if(cl.Size ()  > commandIndex)
	    delete prepInput;
    if(cl.Size ()  > 1+commandIndex)
	    delete prepOutput;

	return 0;
}

//=============================================================================

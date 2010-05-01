//============================================================================
// lvldump.cc:
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

#include <stdio.h>
#include <stdlib.h>
#include <pigsys/assert.hp>
//#include <brender.h>
//#include "pigtool.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#include "level.hp"

#include <cpplib/stdstrm.hp>
//#include <pclib/boolean.hp>
extern char szVersion[];

bool bAnalysis = false;

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine( int argc, char* argv[] )
{
    int index;
	for ( index=1; argv[index] && (*argv[index] == '-'); ++index )
	 {
		if ( strcmp( argv[index], "-analysis" ) == 0 )
			bAnalysis = true;
		else
			{
			switch(*(argv[index]+1))
		 		{
				case 'l':
				case 'L':
					LoadLCFile(argv[index]+2);
					LoadOADFiles(argv[index]+2);
					break;

				case 'p':
				case 'P':
					RedirectStandardStream(argv[index]+2);
					break;

				default:
					cerror << "lvldump Error: Unrecognized command line switch \"" <<
						*(argv[index]+1) << "\"" << endl;
					break;
		 		}
			}
	 }

	if(argc-(index-1) < 2)
	  {
//		char* cp;
//         cp = strrchr( argv[0], '\\');
//         cp++;
		cout << "By Kevin T. Seghetti" << endl;
//		cout << "Usage: " << ((cp) ? cp : argv[0]) << " {switches} <.lvl file name> {<output name>}" << endl;
		cout << "Usage: " << argv[0] << " {switches} <.lvl file name> {<output name>}" << endl;
		cout << "Switches:" << endl;
		cout << "    -analysis" << endl;
		cout << "    -l<.lc filename>" << endl;
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
	return(index);
}

//============================================================================

int
filelen(FILE* fp)
{
	int len;

	fseek(fp,0,SEEK_END);
	len = ftell(fp);
	fseek(fp,0,SEEK_SET);
	return(len);
}

//============================================================================

int
main(int argc,char *argv[])
{
	cout << "lvldump v" << szVersion << endl;
	cout << "Copyright 1995,96,97,98,99,2001 World Foundry Group." << endl;
	int index = ParseCommandLine(argc,argv);

	cprogress << "Loading .LVL file <" << argv[index+0] << ">" << endl;

//	ifstream file(argv[index+0]);
//	if(file.rdstate() != ios::goodbit)
//	 {
//		cerror << "lvldump Error: input .LVL file <" << argv[index+0] << "> not found." << endl;
//		exit(1);
//	 }

	// load entire level into memory
	FILE* fp;
	fp = fopen(argv[index+0],"rb");
	if(!fp)
	 {
		cerror << "lvldump Error: input .LVL file <" << argv[index+0] << "> not found." << endl;
		exit(1);
	 }

	assert(fp);
	long fileSize = filelen(fp);
	void* pLevelData = malloc(fileSize);				// alloc memory for level data file
	assert(pLevelData);
	fread(pLevelData,fileSize,1,fp);				// read entire file
	fclose(fp);

	ostream* out;
	if((argc - index) > 1)
	 {
		cprogress << "Writing output file <" << argv[index+1] << ">"  << endl;
		out = new ofstream(argv[index+1]);
		if(!out)
		 {
			cerr << "Unable to open output file " << argv[index+1] << endl;
			exit(1);
		 }
	 }
	else
		out = &cout;

	*out << "Level Dump created by Lvldump version " << szVersion << endl;
	DumpLevel((_LevelOnDisk*)pLevelData,*out);

    if(out != &cout)
	    delete out;						
	cprogress << "lvldump Done" << endl;
    return 0;
}

//============================================================================

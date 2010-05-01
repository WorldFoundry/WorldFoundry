//============================================================================
// levelcon.cc:
// Copyright(c) 1995 Cave Logic Studios / PF.Magic
// By Kevin T. Seghetti
//============================================================================
// Revision history:
//	1.25	Phil added scaling per objectOnDisk
//	1.24	Phil added ActBoxCA sorting
//	1.23	Phil added COMMONBLOCK support
//
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
//#include <brender.h>
#include "pigtool.h"
#include <iostream.h>
#include <fstream.h>

#include <pclib/stdstrm.hp>

#include "levelcon.hp"
#include "level.hp"

#include "file3ds.hp"

#pragma pack(1);
extern "C" {
#include "3dsftk.h"
};
#pragma pack();

#include "memory.hp"

#ifdef _MEMCHECK_
#include <memcheck.h>
#endif

//=============================================================================

boolean writeNames = boolean::BOOLFALSE;
boolean mergeObjects = boolean::BOOLFALSE;

targetSystem levelconTargetSystem = TARGET_WINDOWS;

char* TARGETNAMES[TARGET_MAX] =
{
	"PSX",
	"SAT",
	"WIN",
	"DOS"
};

//=============================================================================
// returns index of first argv which doesn't contain a '-' switch

int
ParseCommandLine(int argc, char** argv)
{
	for ( int i=1; argv[i] && (*argv[i] == '-'); ++i )
	 {
		switch(*(argv[i]+1))
		 {
			case 't':
			case 'T':
				switch(*(argv[i]+2))
				 {
					case 'W':
					case 'w':
						levelconTargetSystem = TARGET_WINDOWS;
						break;
					case 'D':
					case 'd':
						levelconTargetSystem = TARGET_DOS;
						break;

					case 'S':
					case 's':
						levelconTargetSystem = TARGET_SATURN;
						cout << "Levelcon Error: saturn not supported" << endl;
						break;
					case 'p':
					case 'P':
						levelconTargetSystem = TARGET_PLAYSTATION;
						break;
					default:
						cout <<"Levelcon error: target type " << argv[i]+2 << " not supported" << endl;
						exit(1);
						break;
				 }
				break;
			case 'p':
			case 'P':
				RedirectStandardStream(argv[i]+2);
				break;
			case 'n':
			case 'N':
				writeNames = boolean::BOOLTRUE;
				break;
			case 'm':
			case 'M':
				mergeObjects = boolean::BOOLTRUE;
				break;

			case 'd':
			case 'D':
				if(argc-i == 2)
				 {
				 	QFile3ds* levelFile3ds;
					levelFile3ds = new QFile3ds(argv[i+1]);
					assert(levelFile3ds);
					cout << "Dumping meshes from file " << argv[i+1] << endl;
					levelFile3ds->DumpMeshes(cout);
					delete levelFile3ds;
					exit(0);
				 }
				else
					argc = 0;					// force usage to get printed
				break;

			case 'v':				// verify .prj file
			case 'V':
				if(argc-i == 2)
				 {
				 	QFile3ds* levelFile3ds;
					cout << "Loading 3ds file " << argv[i+1] << endl;
					levelFile3ds = new QFile3ds(argv[i+1]);
					assert(levelFile3ds);
					cout << "closing 3ds file " << endl;
//					levelFile3ds->DumpMeshes(cout);
					delete levelFile3ds;
					exit(0);
				 }
				else
					argc = 0;					// force usage to get printed
				break;

			default:
				cerror << "LevelCon Error: Unrecognized command line switch \"" <<
					*(argv[i]+1) << "\"" << endl;
				break;
		 }
	 }

	if(argc-(i-1) < 3)
	  {
		char* cp;
		cp = strrchr( argv[0], '\\');
		cp++;
		cout << "Usage : " << ((cp) ? cp : argv[0]) << " {switches} <3DS file name> <.lc file name> <output level>" << endl;
		cout << "   Or : " << ((cp) ? cp : argv[0]) << " -d <3DS file name> (to dump meshes)" << endl;
		cout << "   Or : " << ((cp) ? cp : argv[0]) << " -v <3DS file name> (to test 3ds file)" << endl;
		cout << "Switches:" << endl;
		cout << "    -m  Enable merging of static platforms" << endl;
		cout << "    -n  Causes objects.id to be written" << endl;
		cout << "    -t<target initial>, where:" << endl;
		cout << "        p=psx, d=dos, w=win" << endl;
#if DEBUG
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
#endif
		exit(1);
	  }
	return(i);
}

//============================================================================

boolean goodLevel = boolean::BOOLTRUE;

const char names[] = "objects.id";

int32 numPF3s = 0;	// Number of assetIDs generated for .pf3 files

int
realmain(int argc,char *argv[])
{
	cout << "LevelCon V" LEVELCON_VER;
#if DEBUG
	cout <<" Debug";
#else
	cout <<" Release";
#endif
	cout <<  " Copyright (c) 1995,96 Cave Logic Studios / PF. Magic\nAll Rights Reserved.  By Kevin T. Seghetti\n";
#ifdef MUTATE_VECTORS
#error mutate vectors no longer supported
#else
#endif

	int index = ParseCommandLine(argc,argv);

	if(writeNames)
		remove(names);

	QLevel pLevel(argv[index+0],argv[index+1]);

	// tweak hierarchical/relative paths
	pLevel.DoCrossReferences(argv[index+0],argv[index+1],argv[index+2]);							// handle references, and cameras etc.

	DBSTREAM1( cprogress << "Writing level file <" << argv[index+2] << ">"  << endl; )
	assert(goodLevel == boolean::BOOLTRUE);
	pLevel.Save(argv[index+2]);

	// kts do it after so that we write out the merged object names as well
	if(writeNames)
	 {
		char drive[_MAX_DRIVE];
		char path[_MAX_PATH];
		char result[_MAX_PATH];

		_splitpath(argv[index+2],drive,path,NULL,NULL);
		_makepath(result,drive,path,"objects",".id");

		ofstream out(result);
		if(out.rdstate() != ios::goodbit)
		 {
			cerror << "Levelcon Error: cannot create file <" << result << ">" << endl;
			exit(1);
		 }
		pLevel.PrintNameList(out);
	 }

//	DumpFreeMemory(cwarn);
	DBSTREAM3( cprogress << "LevelCon Done" << endl; )
	return(0);
}

//============================================================================

int
main(int argc,char *argv[])
{
//	DumpFreeMemory(cwarn);
	realmain(argc,argv);
//	DumpFreeMemory(cwarn);
	return(0);
}

//============================================================================

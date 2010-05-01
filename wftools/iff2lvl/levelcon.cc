//============================================================================
// levelcon.cc:
// Copyright(c) 1995,96,97,2000,2001,2002 World Foundry Group
// By Kevin T. Seghetti & Phil Torre
//============================================================================
// Revision history:
//	1.25	Phil added scaling per objectOnDisk
//	1.24	Phil added ActBoxCA sorting
//	1.23	Phil added COMMONBLOCK support
//	1.24	WBNIV	Added levelcon(); can be called from Utility
//
//============================================================================

#include "levelcon.hp"
#include <pigsys/pigsys.hp>
#include <cpplib/libstrm.hp>
#include <stdio.h>
#include <stdlib.h>
//#include <iostream.h>
//#include <fstream.h>
#include "level.hp"
//#include "../lib/registry.h"
#include "memory.hp"

//============================================================================

//extern Interface* gMaxInterface;

QLevel* theLevel;	// global pointer so Path can ask things of the Level object

//=============================================================================

bool writeNames = false;
bool mergeObjects = false;

targetSystem levelconTargetSystem = TARGET_WINDOWS;

char* targetNames[TARGET_MAX] =
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
   int i;
	for ( i=1; argv[i] && (*argv[i] == '-'); ++i )
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
						assert(0);
						break;
				}
				break;
			case 'p':
			case 'P':
				RedirectStandardStream(argv[i]+2);
				break;
            case 'l':
            case 'L':
                RedirectLibraryStream(argv[i]+2);
			case 'n':
			case 'N':
				writeNames = true;
				break;
			case 'm':
			case 'M':
				assert(0);				// kts 11/2/97 08:40 merged objects no longer supported
				mergeObjects = true;
				break;

#if 0
			case 'd':
			case 'D':
				if(argc-i == 2)
				{
//				 	QFile3ds* levelFile3ds;
//					levelFile3ds = new QFile3ds(argv[i+1]);
//					assert(levelFile3ds);
//					cout << "Dumping meshes from file " << argv[i+1] << endl;
//					levelFile3ds->DumpMeshes(cout);
//					delete levelFile3ds;
					exit(0);
				}
				else
					argc = 0;					// force usage to get printed
				break;

			case 'v':				// verify .prj file
			case 'V':
				if(argc-i == 2)
				{
//				 	QFile3ds* levelFile3ds;
//					cout << "Loading 3ds file " << argv[i+1] << endl;
//					levelFile3ds = new QFile3ds(argv[i+1]);
//					assert(levelFile3ds);
//					cout << "closing 3ds file " << endl;
//					levelFile3ds->DumpMeshes(cout);
//					delete levelFile3ds;
					exit(0);
				}
				else
					argc = 0;					// force usage to get printed
#endif
				break;

			default:
				AssertMessageBox(0, "Registry \\Recombinant\\World Foundry\\GDK\\max2lvl\\OPTIONS string is bogus:" << endl << *(argv[i]+1));
				break;
		}
	}

	if(argc-(i-1) < 3)
	{
		char* cp;
		cp = strrchr( argv[0], '\\');
        if(cp)
		    cp++;
		cout << "Usage : " << ((cp) ? cp : argv[0]) << " {switches} <exported iff file name> <.lc file name> <output level>" << endl;
//		cout << "   Or : " << ((cp) ? cp : argv[0]) << " -d <3DS file name> (to dump meshes)" << endl;
//		cout << "   Or : " << ((cp) ? cp : argv[0]) << " -v <3DS file name> (to test 3ds file)" << endl;
		cout << "Switches:" << endl;
		cout << "    -m  Enable merging of static platforms" << endl;
		cout << "    -n  Causes objects.id to be written" << endl;
		cout << "    -t<target initial>, where:" << endl;
		cout << "        p=psx, w=win" << endl;
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
        cout << "\t-l<stream initial><stream output>, where:" << std::endl;
        cout << "\t\t<stream initial> can be any of:" << std::endl;
    #define STREAMENTRY(stream,where,initial,helptext) std::cout << "\t\t\t" << initial << "=" << helptext << std::endl;
    #include <libstrm.inc>
    #undef STREAMENTRY
        std::cout << "\t\t<stream output> (same as above)" << std::endl;

#endif
		exit(1);
	}
	return(i);
}

//============================================================================


const char names[] = "objects.id";

int
GetLevelNumberFromCurrentFile()
{
#pragma message ("KTS " __FILE__ ": finish this")
	assert(0);
#if 0
	Interface* ip = GetCOREInterface();
	assert( ip );

	// Level
	TSTR szProjectFilename = ip->GetCurFileName();
	if ( !*szProjectFilename )
	{	// Error -- no level to run
		//Error( "No level file loaded" );
		return -1;
	}

	if ( strnicmp( szProjectFilename, "level", strlen( "level" ) ) != 0 )
	{	// Error
		//Error( "Filename must start with \"level\"" );
		return -1;
	}

	return atoi( szProjectFilename + strlen( "level" ) );
#else
	return 0;
#endif
}

#pragma message ("KTS " __FILE__ ": write a main")
#if 0
extern "C" int
levelcon( const char* szOutputFilename )
{
	Interface* ip = GetCOREInterface();
	assert( ip );

	LVLExport le;	// = new LVLExport;
	//assert( le );
#if MAX_RELEASE < 2000
	int ret = le.DoExport( szOutputFilename, NULL, ip );
#else
	int ret = le.DoExport( szOutputFilename, NULL, ip, 0 );
#endif
	//delete le;
	return ret;
}
#endif

//==============================================================================

RealMalloc* levelconRM;

int
main( int argc, char* argv[] )
{
	cout << "IFF2LVL V0.14" << endl;
	RedirectStandardStream("sn");		// kts fix this, stats shouldn't be so wordy		
	int index = ParseCommandLine( argc, argv );



	levelconRM = new RealMalloc MEMORY_NAMED( ("levelcon") );
	assert(ValidPtr(levelconRM));
	levelconRM->Validate();

	if ( writeNames )
		remove( names );

	QLevel pLevel(argv[index+0], argv[index+1] /*, theScene*/ );

	// tweak hierarchical/relative paths
	pLevel.DoCrossReferences(argv[index+0],argv[index+1],argv[index+2]);							// handle references, and cameras etc.

	DBSTREAM1( cprogress << "Writing level file <" << argv[index+2] << ">"  << endl; )
	pLevel.Save(argv[index+2]);

	// kts do it after so that we write out the merged object names as well
	if(writeNames)
	{
		char drive[_MAX_PATH];
		char path[_MAX_PATH];
		char result[_MAX_PATH];

		_splitpath(argv[index+2],drive,path,NULL,NULL);
		_makepath(result,drive,path,"objects",".id");

		ofstream out(result);
		AssertMessageBox(out.good(), "Cannot create file <" << result << ">");
		pLevel.PrintNameList(out);
	}

	DBSTREAM3( cprogress << "LevelCon Done" << endl; )
	return(0);
}

//============================================================================

ostream& operator<<(ostream& s, const Point3 &point)
{
	s << "x <" << point.x << "> y <" << point.y << "> z <" << point.z << "> " << endl;
	return s;
}

//============================================================================

ostream& operator<<(ostream& s, const Matrix3 &mat)
{
	s << mat.GetRow(0);
	s << mat.GetRow(1);
	s << mat.GetRow(2);
	s << mat.GetRow(3);
	return s;
}

//============================================================================

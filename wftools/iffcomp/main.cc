//==============================================================================
// main.cc: Copyright (c) 1996-3003, World Foundry Group  
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

#include <pigsys/pigsys.hp>
#include <version/version.hp>
#include <string>
#include <iostream>
#include <recolib/command.hp>
#include <cpplib/stdstrm.hp>
#include <cpplib/libstrm.hp>

//using namespace std;

int iffcomp( const char* _szInputFile, const char* _szOutputFile );
bool bVerbose = false;
bool bQuiet;
bool bBinary = true;
extern int yydebug;			/*  nonzero means print parse trace	*/

char szSwitchOuputFile[] = "-o=";
const char* _szInputFile = NULL;
const char* _szOutputFile = NULL;

//==============================================================================

int
ParseCommandLine(const CommandLine& cl)
{
    unsigned int commandIndex;
	for ( commandIndex=1; commandIndex < cl.Size() && (cl[commandIndex][0] == '-'); ++commandIndex )
	{
		std::string command = cl[commandIndex];
		assert(command[0] == '-');
		DBSTREAM2( cdebug << "command line option <" << command << "> Found" << std::endl; )
		std::string restOfCommand = std::string(command.substr(2,command.length()-2));
		switch(command[1])
		{
#if SW_DBSTREAM > 0
			case 'p':
			case 'P':
				RedirectStandardStream(restOfCommand.c_str());
				break;
			case 'l':
			case 'L':
				RedirectLibraryStream(restOfCommand.c_str());
				break;
#endif
			case 'v':
				bVerbose = true; 
            	yydebug = 1;			/*  nonzero means print parse trace	*/
				break;
			case 'q' :
				bQuiet = true;
				break;

			case 'a' :
				if( strcmp( command.c_str(), "-ascii" ) == 0 )
					bBinary = false;
				else
				cerror << "iffcomp Error: Unrecognized command line switch \"" <<
					command << "\"" << std::endl;
				break;

			case 'b' :
				if( strcmp( command.c_str(), "-binary" ) == 0 )
					bBinary = true;
				else
					cerror << "iffcomp Error: Unrecognized command line switch \"" <<
						command << "\"" << std::endl;
				break;
			case 'o' :
				if ( strncmp( command.c_str(), szSwitchOuputFile, strlen( szSwitchOuputFile ) ) == 0 )
					_szOutputFile = strdup(command.c_str() + strlen( szSwitchOuputFile ));
				else
					cerror << "iffcomp Error: Unrecognized command line switch \"" <<
						command << "\"" << std::endl;
				break;

			default:
				cerror << "iffcomp Error: Unrecognized command line switch \"" <<
					command << "\"" << std::endl;
				break;
		 }
	 }

	if(cl.Size()-(commandIndex-1) < 2)
	{
		std::cout << "iffcomp v" << szVersion
			<< "  Copyright 1997-2003 World Foundry Group."
			<< "  All Rights Reserved." << std::endl;
		std::cout << "By William B. Norris IV" << std::endl;
		std::cout << "Usage: iffcomp {-<switches>} <infile>.txt.iff" << std::endl;
		std::cout << "Switches:" << std::endl;
		std::cout << "    -ascii  Sets output file type to ascii" << std::endl;
		std::cout << "    -binary Sets output file type to binary" << std::endl;
		std::cout << "    -v verbose mode, lots of lex/yacc output" << std::endl;
		std::cout << "    -q quiet mode, suppresses most printing" << std::endl;
		std::cout << "    -o=<outfile> Sets output filename. If not specified test.wf is used" << std::endl;
		std::cout << "    -p<stream initial><stream output>, where:" << std::endl;
		std::cout << "        <stream initial> can be any of:" << std::endl;
		std::cout << "            w=warnings (defaults to standard err)" << std::endl;
		std::cout << "            e=errors (defaults to standard err)" << std::endl;
		std::cout << "            f=fatal (defaults to standard err)" << std::endl;
		std::cout << "            s=statistics (defaults to null)" << std::endl;
		std::cout << "            p=progress (defaults to null)" << std::endl;
		std::cout << "            d=debugging  (defaults to null)" << std::endl;
		std::cout << "        <stream output> can be any of:" << std::endl;
		std::cout << "            n=null, no output is produced" << std::endl;
		std::cout << "            s=standard out" << std::endl;
		std::cout << "            e=standard err" << std::endl;
		std::cout << "            f<filename>=output to filename" << std::endl;
		std::cout << "\t-l<stream initial><stream output>, where:" << std::endl;
		std::cout << "\t\t<stream initial> can be any of:" << std::endl;
#define STREAMENTRY(stream,where,initial,helptext) std::cout << "\t\t" << initial << "=" << helptext << std::endl;
#include <libstrm.inc>
#undef STREAMENTRY
		std::cout << "\t<stream output> (same as above)" << std::endl;

		exit(1);
	}
	return(commandIndex);
}

//==============================================================================


int
main( int argc, char* argv[] )
{
	CommandLine cl(argc,argv);
	unsigned int commandIndex = ParseCommandLine(cl);

	_szInputFile = argv[commandIndex];

	if ( !_szOutputFile )
	{
		_szOutputFile = "test.wf";
	}
	return iffcomp( _szInputFile, _szOutputFile );
}
